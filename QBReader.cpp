#include <windows.h>
#include <stdio.h>


// defines
#define GET_ULONG	(unsigned long)*(unsigned long*)&
#define GET_USHORT	(unsigned short)*(unsigned short*)&

// variables
char g_fileName[MAX_PATH]= "";
unsigned long **g_keyID;
char **g_keyName;
unsigned short g_keyNum = 0;
unsigned char *g_qbData;
unsigned long g_qbSize;

// functions
bool OnLoadFile();
bool OnLoadKeys();
bool OnSaveQB();


int main(int argc, char* argv[])
{

	printf("       ___\n");
	printf("  __  /\\__\\\n");
	printf(" /\\ \\/ /  /\n");
	printf(" \\ \\/ /  /      ________\n");
	printf("  \\  /  / - - -/        \\-\n");
	printf("  / /  / oshua  SOFTWARE\n");
	printf("  \\/__/ - - - -\\________/-\n");
	printf("\n");
	printf("***************************\n");
	printf("     THUG2 QB Reader\n");
	printf("***************************\n");
	printf("Author: yosh64/defeat0r\n");
	printf("Website: http://jmn-web.tk\n\n");

	printf("<- description\n");
	printf("Extract any THUG2 QB Script\n");
	printf("file into a readable format.\n\n");


	// check runtime parameter count...
	if(argc < 2) {

		// open file dialog
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = "QB File (*.qb)\0*.qb\0\0";
		ofn.lpstrFile = g_fileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrInitialDir = NULL;

		if(!GetOpenFileName(&ofn)) {
			printf("ERROR: No file selected!\n\n");
			system("PAUSE");
			return 0;
		}
	}
	else {
		strcpy(g_fileName, argv[1]);
	}

	// varify arguments
	if(argc > 2) {
		printf("+ USAGE: QBReader.exe [input.qb]\n");
		return 0;
	}


	// load qb data
	OnLoadFile();
	
	// load key ids
	if(!OnLoadKeys()) {
		printf("ERROR: Unable to load Key Data!\n\n");
		system("PAUSE");
		return 0;
	}

	// tests
	printf("<- testing\n");
	printf("QB File Size: %i bytes\n", g_qbSize);
	printf("Number of Keys: %i \n", (g_keyNum-1));
	printf("Key Name [0]: %s \n\n", g_keyName[0]);

	// set output filename
	short fnLen = strlen(g_fileName);
	g_fileName[fnLen-1] = 't';
	g_fileName[fnLen-2] = 'x';
	g_fileName[fnLen-3] = 't';
	g_fileName[fnLen-4] = '.';

	// save qb
	OnSaveQB();

	// free memory
	delete[] g_keyID;
	delete[] g_keyName;
	delete[] g_qbData;


	// pause at end
	system("PAUSE");

	return 0;
}

bool OnLoadFile()
{

	FILE *inFile;
	inFile = fopen(g_fileName, "rb");


	// check if file was opened
	if(! inFile) {
		printf("ERROR: Unable to open file to load.");
		return NULL;
	}

	// get file size
	fseek(inFile, 0, SEEK_END);
    g_qbSize = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);

	// allocate memory
	g_qbData = new unsigned char [g_qbSize];

	// read data
	fread(g_qbData, 1, g_qbSize, inFile);
	fclose(inFile);

	return true;
}


bool OnLoadKeys()
{

	unsigned long keyOffset;
	unsigned long lp, lp2, cnt=0;

	// Values before declearation...
	// 01 04 01 2B
	// 01 06 01 2B
	// 01 24 01 2B
	// 00 06 01 2B
	// 21 01 00 2B
	// 80 3E 01 2B
	// 23 3C 01 2B
	// CE 00 00 2B
	// 01 2B and 00 2B <- just need these???


	// seek to start of key declaration data
	for(lp=0;lp<g_qbSize-4;lp++) {

		if(GET_USHORT g_qbData[lp] == 0x2B01 || GET_USHORT g_qbData[lp] == 0x2B00) {
			keyOffset = lp+1;
			break;
		}
	}

	// check if key data offset was found
	if(keyOffset == 0) return false;


	// find number of keys
	for(lp=keyOffset;lp<g_qbSize;lp++) if(g_qbData[lp] == 0x2B) lp+=5, g_keyNum++;

	// allocate memory
	g_keyID = new unsigned long *[g_keyNum];
	g_keyName = new char *[g_keyNum];

	lp = keyOffset;
	while(lp < g_qbSize) {

		if(g_qbData[lp] == 0x2B) { // 0x2B before each key

			// set key id
			g_keyID[cnt] = (unsigned long*)&g_qbData[lp+1];

			// set key name
			lp += 5; // goto name
			g_keyName[cnt] = (char*)&g_qbData[lp]; // set pointer to string

			while(g_qbData[lp] != 0x00) lp++;
			lp++, cnt++;
		}
		else break; // lp++ ???
	}

	return true;
}


bool OnSaveQB()
{

	unsigned long lp, lp2;
	short inc = 0;

	char strOut[512];
	int intOut = 0;

	FILE *outFile;
	outFile = fopen(g_fileName, "wt");

	// check if file was opened
	if(!outFile) {
		printf("ERROR: Unable to open file to save.");
		return false;
	}

	// output file header
	fprintf(outFile, "       ___\n  __  /\\__\\\n /\\ \\/ /  /\n \\ \\/ /  /\n  \\  /  /_ _ _ /¯ ¯ ¯ ¯ \\ _\n  / /  / oshua  SOFTWARE \n  \\/  /  ¯ ¯ ¯ \\_ _ _ _ / ¯\n   ¯¯¯  \n-----------------------------\n+++ THUG2 QB READER\nVersion: 0.5\nWebsite: http://jmn-web.tk\nTHPS Alias: Defeat0r\n\n\n---------- QB START\n\n");


	// loop through qb
	for(lp=0;lp<g_qbSize;lp++) {

		inc = 0;

		switch(g_qbData[lp])
		{
			case 0x03: { // begin section

				fprintf(outFile, "%08X:%02X ** BEGIN SECTION ** \n", lp, g_qbData[lp]);
				break;
			}

			case 0x04: { // end section

				fprintf(outFile, "%08X:%02X ** END SECTION ** \n", lp, g_qbData[lp]);
				break;
			}

			case 0x05: { // begin array

				fprintf(outFile, "%08X:%02X ** BEGIN ARRAY ** \n", lp, g_qbData[lp]);
				break;
			}

			case 0x06: { // end array

				fprintf(outFile, "%08X:%02X ** END ARRAY ** \n", lp, g_qbData[lp]);
				break;
			}

			case 0x47: { // ???

				fprintf(outFile, "%08X:%02X %02X \n", lp, g_qbData[lp], g_qbData[lp+1]);
				inc++;
				break;
			}

			case 0x48: { // ???

				fprintf(outFile, "%08X:%02X %02X %02X %02X \n", lp, g_qbData[lp], g_qbData[lp+1], g_qbData[lp+2], g_qbData[lp+3]);
				inc += 2;
				break;
			}

			// 0x48

		
			case 0x2B: { // declaration

					lp2 = 0;

					// loop through string
					while(g_qbData[(lp+5)+lp2] != 0x00) {

						strOut[lp2] = g_qbData[(lp+5)+lp2];
						lp2++;
					}
					strOut[lp2] = '\0'; // null terminator
					fprintf(outFile, "%08X:%02X %02X %02X %02X %02X :: %s \n", lp, g_qbData[lp], g_qbData[lp+1], g_qbData[lp+2], g_qbData[lp+3], g_qbData[lp+4], strOut);
					inc += (lp2+5);
					break;
			}

			case 0x16: { // key id

				// lookup key id
				for(lp2=0;lp2<g_keyNum;lp2++) {

					// compare key ids
					if(GET_ULONG g_qbData[lp+1] == *g_keyID[lp2]) {

						fprintf(outFile, "%08X:%02X %02X %02X %02X %02X -> %s \n", lp, g_qbData[lp], *g_keyID[lp2], g_keyName[lp2]);
					}
				}

				inc += 4;
				break;
			}

		
			case 0x07: { // set something ???

				// output 0x07 line
				fprintf(outFile, "%08X:%02X = \n", lp, g_qbData[lp]);
				// 0x1B or 0x1C (string)??? 0x17 (int) ???
				break;
			}


			case 0x1B: // string
			case 0x1C: {

				lp2 = 0;

				// loop through string...
				while(g_qbData[(lp+5)+lp2] != 0x00) {

					strOut[lp2] = g_qbData[(lp+5)+lp2];
					lp2++;
				}

				strOut[lp2] = '\0'; // null terminator
				fprintf(outFile, "%08X:%02X %02X %02X %02X %02X :: %s \n", lp, g_qbData[lp], g_qbData[lp+1], g_qbData[lp+2], g_qbData[lp+3], g_qbData[lp+4], strOut);
				inc += (lp2+5);
				break;
			}

			case 0x17: { // integer

				intOut = GET_ULONG g_qbData[lp+1];
				fprintf(outFile, "%08X:%02X %02X %02X %02X %02X :: %i \n", lp, g_qbData[lp], g_qbData[lp+1], g_qbData[lp+2], g_qbData[lp+3], g_qbData[lp+4], intOut);					
				inc += 4;
				break;
			}


			default: { // nothing??? 0x00, 0x01, 0x0E, 0x23, 0x28, 0x2D, 0x39, 0x42, 0x2C, 0x32

				fprintf(outFile, "%08X:%02X \n", lp, g_qbData[lp]);
			}
		}

		lp += inc;
	}


	// output file footer...
	fprintf(outFile, "\n---------- QB END\n\n\n*Thanks Sporadic, your qbConsole was helpful ;)");
	fclose(outFile);

	return true;
}