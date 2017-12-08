#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstdint>
#include "Line.h"

using namespace std;

uint8_t crc = 0;

long datacount = 0;


void print_data( Line *line, ofstream *of, int size , int width )
{
    char buff[35];
    int len;
	char* template_ ;
 
    if ( line == nullptr )
    {
        len = size;
    }
    else
    {
        len = line->bytecount;
        sprintf( buff,  "/* 0x%04X */\n\t", line->address );
        *of << ( string )buff;
    }

		uint32_t data;
		
    for ( int i = 0; i < len; i++ )
    {
	data = 0;
		crc ^= data;
		
		if( width == 8 )
		{
			data = ( line == nullptr ) ? 0xFF : ( uint8_t )line->data[i];
			template_ = (char *) "0x%02X, ";
			datacount++;
		}
		
		if( width == 16 )
		{
			for ( int j = 0x1; j < 0x10000 ; i++ , j*=0x100 )
			{
				/* little endian */
				data += ( line == nullptr ) ? 0xFF : (( uint8_t )line->data[i])*j;
				datacount++;
			}
			template_ = (char *) "0x%04X, ";
			i--;
		}
		
		if( width == 32 )
		{
			for ( uint64_t j = 0x1; j < 0x100000000 ; i++ , j*=0x100 )
			{
				/* little endian */
				data += ( line == nullptr ) ? 0xFF : (( uint8_t )line->data[i])*j;
				datacount++;
			}
			template_ = (char *) "0x%08X, ";
				i--;
				
		}
		 
        sprintf( buff,  template_ , data );
        *of << ( string )buff;
         
         if ( !( datacount % 8 ) )
        {
            //printf("\n");
            *of << "\n\t";
        }  
    }
}


/* main entry point */
int main( int argc, char ** argv )
{
    if ( argc < 3 )
    {
        cout << "usage: " << argv[0] << " <hexfile.hex> <output.c> <width>";
		cout << "       <width> data width of each element of the array: 8, 16, 32 ";
        return 1;
    }

    ifstream ifile;
    ofstream ofile;
    string l;

    int last_address = 0;
    int last_type = 0xFF;
    int last_size = 0;
	int data_width ;
	
    string width;
    string ipath = argv[1];
    string opath = argv[2];

    if ( argc == 4 )
    {
        data_width = stoi( argv[3] );
        switch( data_width )
        {
            case 16:
                width = "uint16_t";
                break;
            case 32:
                width = "uint32_t";
                break;
            case 8:
            default:
                width = "uint8_t";
                break;
        }

    }
    else
    {
        width = "uint8_t";
    }

    cout << "opening " << ipath << "...\n";
    ifile.open( ipath, ios::binary | ios::in );
    cout << "opening " << opath << "...\n";
    ofile.open( opath, ios::out | ios::trunc );

    if ( ifile.is_open() && ofile.is_open() )
    {
        ofile << "#include <stdint.h>\n\n";
       // ofile << "#pragma pack(1)\n\n";
        ofile << "const " << width << " hex_image[] = \n";
        ofile << "{\n\t";
        
		while ( getline( ifile, l ) )
        {
            //cout << stoi(line.substr(1, 2), nullptr, 16) << "\n";
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

#if 1
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

                char buff[100];

                switch ( line->type )
                {
                    case 0x00: // data type
                        if ( last_type == 0x04 || last_type == 0x00 )
                        {
                            int blank = line->address - ( last_address + last_size );

                            if ( blank > 0 )
                            {
                                print_data( nullptr, &ofile , blank , data_width );
                            }

                            print_data( line, &ofile , 0 , data_width);
                        }
                        else
                        {
                            sprintf( buff, "/* address: 0x0000XXXX */\n\t" );
                            ofile << ( string )buff;
                            print_data( line, &ofile, 0 , data_width);
                        }
                        break;

                    case 0x01: // end of file
                        sprintf( buff,  "/* END OF DATA */\n\t" );
                        ofile << ( string )buff;
                        break;

                    case 0x02:
                        // TODO: for now do nothing
                        break;

                    case 0x03:
                        // TODO: for now do nothing
                        break;

                    case 0x04:
                        sprintf( buff,  "/* address: 0x%02X%02XXXXX */\n\t",  ( uint8_t )line->data[0], ( uint8_t )line->data[1] );
                        ofile << ( string )buff;
                        break;

                    case 0x05:
                        // TODO: for now do nothing
                        break;

                    default:
                        break;
                }

                last_address 	= line->type == 0x00 ? line->address : last_address;
                last_type 		= line->type;
                last_size 		= line->type == 0x00 ? line->bytecount : last_size;
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
        ifile.close();
        ofile.close();
    }
    else
    {
        cout << "Cannot open file!";
        return 1;
    }
    cout << "\nFinished. Total : " << datacount << " bytes, crc = " << hex << ( int )crc;
    //while (1);
    return 0;
}

