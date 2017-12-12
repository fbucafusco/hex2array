#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstdint>

#include "Line.h"

#ifdef _WIN32
#include "stdafx.h"
#define SPRINTF sprintf
#else
#define SPRINTF sprintf
#endif

#define DEBUG 	0

/*flags*/
#define HEX2ARRAY_FILL_BLANKS 0x00000001


/*compilers*/
#define HEX2ARRAY_COMPILER_NONE	0
#define HEX2ARRAY_GCC 			1
#define HEX2ARRAY_ARMCC			2
#define HEX2ARRAY_IAR			3


using namespace std;

uint32_t crc = 0;

typedef struct
{
    char*   data_type;
    uint8_t data_width;
    uint8_t line_elem;
    uint8_t flags;
    uint8_t compiler;
} tModifier;

long datacount = 0;


void print_data( Line *line, ofstream *of, int size, tModifier* mod )
{
    char buff[35];
    int len;
    char* template_ ;
    void *absolute_addess;
    uint8_t char_data;

    if ( line == nullptr )
    {
        len = size;
    }
    else
    {
        len = line->bytecount; 
    }

    uint32_t data;

    *of << "\t";

    for ( int i = 0; i < len; i++ )
    {
        data = 0;
        crc ^= data;

        if( mod->data_width == 8 )
        {
            char_data = ( line == nullptr ) ? 0xFF : ( uint8_t )line->data[i];
            data = char_data;
            template_ = ( char * ) "0x%02X, ";
            datacount++;
            crc ^= char_data;
        }

        if( mod->data_width == 16 )
        {
            for ( int j = 0x1; j < 0x10000 ; i++, j*=0x100 )
            {
                char_data = ( line == nullptr ) ? 0xFF : ( ( uint8_t )line->data[i] );

                /* little endian */
                data += char_data*j;
                datacount++;
                crc ^= char_data;
            }
            template_ = ( char * ) "0x%04X, ";
            i--;
        }

        if( mod->data_width == 32 )
        {
            for ( uint64_t j = 0x1; j < 0x100000000 ; i++, j*=0x100 )
            {
                char_data = ( line == nullptr ) ? 0xFF : ( ( uint8_t )line->data[i] );

                /* little endian */
                data += char_data*j;
                datacount++;
                crc ^= char_data;
            }

            template_ = ( char * ) "0x%08X, ";

            i--;
        }

        SPRINTF( buff,  template_, data );

        *of << ( string )buff;

        if( !( datacount % ( 4*128/mod->data_width ) ) &&  ( line == nullptr ) )
        {
            *of << "\n";
        }
    }
    *of << "\n";
}


void evaluate_modifiers( char* modifier, tModifier* out )
{
    if( memcmp( "-w", modifier,strlen( "-w" ) )==0 )
    {
        uint8_t dw;

        dw = stoi( &modifier[strlen( "-w" )] );

#if DEBUG==2
        SPRINTF( "parseo width %s %d\n",  modifier, dw );
        //cout <<  "parseo width %s %d\n"  << modifier << dw <<"\n";
#endif
        switch( dw )
        {
            case 16:
                out->data_width = 16;
                out->data_type  = "uint16_t";
                break;
            case 32:
                out->data_width = 32;
                out->data_type  = "uint32_t";
                break;
            case 8:
            default:
                out->data_width = 8;
                out->data_type  = "uint8_t";
                break;
        }
    }


    if( memcmp( "-b", modifier,strlen( "-b" ) )==0 )
    {
        out->flags |= HEX2ARRAY_FILL_BLANKS;
    }

    if( memcmp( "-c", modifier,strlen( "-c" ) )==0 )
    {
        uint8_t compiler;

        compiler = stoi( &modifier[strlen( "-c" )] );

        out->compiler = compiler;
    }


    if( memcmp( "-e", modifier,strlen( "-e" ) )==0 )
    {
        uint8_t elements;

        elements = stoi( &modifier[strlen( "-e" )] );

#if DEBUG==2
        printf( "parseo width %s %d\n",  modifier, dw );
#endif
    }
}

void print_array_declaration( ofstream *of, int index, tModifier* mod, uint32_t base_address )
{
    char buff[200];
    char buff2[200];

    switch( mod->compiler )
    {
        case HEX2ARRAY_GCC:
            //todo
            break;
        case HEX2ARRAY_ARMCC:
            sprintf( buff2, "__attribute__ ( ( at( 0x%X ) ) )", base_address );
            break;

        case HEX2ARRAY_IAR:
            //todo
            break;
        case HEX2ARRAY_COMPILER_NONE:
            buff2[0]='\0';
        default:
            break;
    }

    sprintf( buff, "const %s hex_image_%03u[] %s= \n{\n", mod->data_type, index, buff2 );

    *of << buff;
    //   *of << "\t";
}

/* main entry point */
int main( int argc, char ** argv )
{
    int i;

    cout << "hex2array v0.1\n\n";


    if ( argc < 3 )
    {
        cout << "usage: " << argv[0] << " <hexfile.hex> <output.c> <modifiers>";
        cout << "       -wXX XX = data width of each element of the array: 8, 16, 32 ";
        cout << "       -b      = adds pradding (0xFF) from the first section to the first data ";
        cout << "       -cX  X  = target compiler selection 0: none";
        cout << "                                           1: gcc";
        cout << "                                           2: armcc";
        cout << "                                           3: iar";

        return 1;
    }

    ifstream ifile;
    ofstream ofile;
    string l;

    int last_base = 0;
    int last_address = 0;
    int last_type = 0xFF;
    int last_size = 0;
    uint8_t base_count; //amount of intel hex 04 commands
    int bases[200];
    char buff[100];


    //	int data_width ;

    base_count = 0;

    //string width;
    string ipath = argv[1];
    string opath = argv[2];

    printf( "%s\n",argv[1] );
    printf( "%s\n",argv[2] );

    tModifier modifiers;

    /* load default values */
    modifiers.data_width	= 8;
    modifiers.flags			= 0;
    modifiers.data_type		= "uint8_t";
    modifiers.line_elem		= 16/modifiers.data_width;
    modifiers.compiler      = HEX2ARRAY_COMPILER_NONE;

    if( argc > 3 )
    {
        for( i=3; i<argc; i++ )
        {
            evaluate_modifiers( argv[i], &modifiers );
        }
    }

    cout << "opening " << ipath << "...\n";
    ifile.open( ipath, ios::binary | ios::in );

    cout << "opening " << opath << "...\n";
    ofile.open( opath, ios::out | ios::trunc );

    if ( ifile.is_open() && ofile.is_open() )
    {
        ofile << "#include <stdint.h>\n\n";
        // ofile << "#pragma pack(1)\n\n";
        // ofile << "const " << modifiers.data_type << " hex_image[] = \n";
        // ofile << "{\n\t";

        while ( getline( ifile, l ) )
        {
            int bytecount = stoi( l.substr( 1, 2 ), nullptr, 16 );

            Line *line = new Line( bytecount );

            if ( line != nullptr )
            {
                // get address
                line->address = stoi( l.substr( 3, 4 ), nullptr, 16 );

                // get type
                line->type = stoi( l.substr( 7, 2 ), nullptr, 16 );

                // get data
                for ( int i = 0; i < bytecount; i++ )
                {
                    line->data[i] = ( char )stoi( l.substr( 9 + ( i * 2 ), 2 ), nullptr, 16 );
                }

                // get checksum
                line->checksum = stoi( l.substr( 9 + ( bytecount * 2 ), 2 ), nullptr, 16 );

#if DEBUG==1
                cout << "bytecount: " << setfill( '0' ) << setw( 2 ) << std::hex << line->bytecount << "\n";
                cout << "address: " << setfill( '0' ) << setw( 2 ) << std::hex << line->address << "\n";
                cout << "type: " << setfill( '0' ) << setw( 2 ) << std::hex << line->type << "\n";
                cout << "data: [";

                for ( int i = 0; i < line->bytecount; i++ )
                {
                    cout << setfill( '0' ) << setw( 2 ) << std::hex << ( int )line->data[i];
                    if ( i != bytecount-1 )
                    {
                        cout << "|";
                    }
                }
                cout << "]\n";
                cout << setfill( '0' ) << setw( 2 ) << "checksum: " << std::hex << line->checksum << "\n";
#endif

                switch ( line->type )
                {
                    case 0x00: // data type

                        if ( last_type == 0x04 )
                        {
                            int last_base_;
							 
                            if( modifiers.flags&HEX2ARRAY_FILL_BLANKS )
                            {
                                int blank = line->address - ( last_address + last_size );

                                last_base_ = last_base;
                                bases[base_count] = last_base;

                                if( bases[base_count]!=bases[base_count-1]  )
                                {
                                    if( base_count>0 )
                                    {
                                        ofile<<"}\n";
                                    }

                                    print_array_declaration(  &ofile, base_count, &modifiers, last_base );
                                }

								if(blank)
                                print_data( nullptr, &ofile, blank , &modifiers );
                            }
                            else
                            {
                                bases[base_count] = last_base + line->address;

                                if( base_count>0 )
                                {
                                    ofile<<"}\n";
                                }

                                print_array_declaration(  &ofile, base_count, &modifiers, last_base + line->address );
                            }

                            print_data( line, &ofile, 0, &modifiers );
							 
                            base_count++;
                        }
                        else if (  last_type == 0x00 )
                        {
                            print_data( line, &ofile, 0, &modifiers );
                        }
                        else
                        {
                           
                        }
                        break;

                    case 0x01: // end of file
                        SPRINTF( buff,  "/* END OF DATA */\n" );
                        ofile << ( string )buff;
                        break;

                    case 0x02:
                        // TODO: for now do nothing
                        break;

                    case 0x03:
                        // TODO: for now do nothing
                        break;

                    case 0x04:
                        last_base =  ( ( uint32_t )line->data[1] + ( ( uint32_t )line->data[0]<<8 ) ) <<16  ;
						 
                        break;

                    case 0x05:
                        // TODO: for now do nothing
                        break;

                    default:
                        break;
                }

                last_address 	= line->type == 0x00 ? line->address : 0 ;
                last_type 		= line->type;
                last_size 		= line->type == 0x00 ? line->bytecount : 0;
            }
            else
            {
                cout << "Unable to allocate line memory!";
                return -1;
            }
        }
        ofile << "};\n\n";

        // ofile << "#pragma pack()\n\n";

        ofile << "int hexdata_size = " << datacount << ";\n";
        ofile << "uint8_t hexdata_crc = 0x" << hex << ( int )crc << ";\n";

        uint8_t added = 0;
        for( i=0; i<base_count; i++ )
        {
            SPRINTF( buff,"const uint32_t base_address_%03u = 0x%08X;\n", added, bases[i] );
            ofile << ( string )buff;
            added++;
            /* remove repeated consecutive bases */
            /*  if( i>0 && bases[i]!=bases[i-1] )
              {

              }

              if( i==0 )
              {
                  SPRINTF( buff, "const uint32_t base_address_%03u = 0x%08X;\n", added, bases[i] );
                  ofile << ( string )buff;
                  added++;
              }*/
        }

        ifile.close();
        ofile.close();
    }
    else
    {
        cout << "Cannot open file!";
        return 1;
    }

    cout << "\nFinished. Total : " << datacount << " bytes, crc = " << hex << ( int )crc;

    return 0;
}

