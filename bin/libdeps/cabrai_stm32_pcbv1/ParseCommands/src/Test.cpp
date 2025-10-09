/**
 * Example using ParseCommands library.
*/
#include <Arduino.h>

#include <ParseCommands.h>

void PCmd_EventCallback( int event );

// Declare functions.
void CmdComment( int argc, char *argv[] );
void CmdTest( int argc, char *argv[] );
void CmdTest2( int argc, char *argv[] );
void CmdGetLast( int argc, char *argv[] );
void CmdSetComment( int argc, char *argv[] );
void CmdGetComment( int argc, char *argv[] );

void display_freeram();
int freeRam();


// List of commands with callback to functions.
struct pcmd_command_t commandList[] = {
	// command, callback function
	";", CmdComment,
	"test", CmdTest,
	"test2", CmdTest2,
	"get", CmdGetLast,
	"setComment", CmdSetComment,
	"getComment", CmdGetComment,
	"cmt", CmdComment,
	NULL, NULL				// END OF LIST (NEEDED)
};

/**
 * Instatiate an ParseCommands.
*/
ParseCommands pCmd;
// ParseCommands pCmd( commandList );
// ParseCommands pCmd( commandList, 32 );
// ParseCommands pCmd( commandList, 48, 5 );

void setup()
{
  // put your setup code here, to run once:
	boolean cmdOk;

	Serial.begin( 115200 );

	pCmd.begin( commandList, 128, 10 );		// Init ParseCommands.
	pCmd.eventHandler( PCmd_EventCallback );

    delay( 500 );
    Serial.println( "ParseCommands Example" );

    // Test string commands.s
	cmdOk = pCmd.doCommand( "test \"121\" 2 3   " );
	delay( 1000 );
	cmdOk = pCmd.doCommand( "test \"1 1\" 2 3" );
	// delay( 1000 );
	cmdOk = pCmd.doCommand( "testx 11 \\ \" 2 3" );
	delay( 1000 );

	if( !cmdOk )
		Serial.printf( "Error: %i - %s\n", pCmd.getError(), pCmd.getErrorText() );

	char cmd[20];
	sprintf( cmd, "test %i %i done", 5, 9);
	pCmd.doCommand( cmd );
}

void loop()
{
 
	bool err = true;
	if( Serial.available() ) {err = pCmd.read( Serial.read() ); }

	// if( !err)
	// {	
	// 	Serial.print( "Error code: ");
	// 	Serial.print( pCmd.getError() );
	// 	Serial.print( " - " );
	// 	Serial.println(pCmd.getErrorText() );
	// 	// display_freeram();

	// 	err = true;
	// }
}

void PCmd_EventCallback( int event )
{
	switch( event )
	{
		case PCMD_INPUT_CHAR_EVT:
			// Serial.print( pCmd.getLastCharRead() );		// Echo.
			break;

		case PCMD_READ_COMMAND_EVT:
			Serial.printf( "Read command: %s\n", pCmd.getLastCommand() );
			break;

		case PCMD_DO_COMMAND_EVT:
			Serial.printf( "Do command: %s\n", pCmd.getLastCommand() );
			break;

		case PCMD_ERROR_EVT:
			Serial.printf("ParseCommands - Error: %i %s\n", pCmd.getError(), pCmd.getErrorText() );
			break;
	}
}

/**
 * Callback for the command 'Set comment string'.
 */
void CmdSetComment(int argc, char *argv[] )
{
	Serial.printf( "CmdSetComment() : argc: %i\n", argc );

	if( argc != 0 )
	{
		for( int i=0; i<argc; i++ )
		{
			Serial.print( "   Parameter " );
			Serial.print( i );
			Serial.print( " : --");
			Serial.print( argv[i] );
			Serial.println( "--");
		}
	}

	if( pCmd.getError() != 1 )
	{
		Serial.print( "Error:" );
		Serial.println( pCmd.getError());
	}

	bool ret = pCmd.setCommentString( argv[0] );
	Serial.print( "Set comment string return: " );
	Serial.println( ret );
}

/**
 * Callback for the command 'Get comment string'.
 */
void CmdGetComment( int argc, char *argv[] )
{
	Serial.printf( "Comment String: ---%s---\n", pCmd.getCommentString() );

}

/**
 * Callback for the command 'comment'.
 */
void CmdComment(int argc, char *argv[] )
{
	Serial.printf( "CmdComment() : argc: %i\n", argc );

	if( argc != 0 )
	{
		for( int i=0; i<argc; i++ )
		{
			Serial.print( "   Parameter " );
			Serial.print( i );
			Serial.print( " : --");
			Serial.print( argv[i] );
			Serial.println( "--");
		}
	}

	if( pCmd.getError() != 1 )
	{
		Serial.print( "Error:" );
		Serial.println( pCmd.getError());
	}

}



/**
 * Callback for the command 'test'.
 */
 void CmdTest(int argc, char *argv[] )
{
	Serial.printf( "CmdTest() : argc: %i\n", argc );

	if( argc != 0 )
	{
		for( int i=0; i<argc; i++ )
		{
			Serial.print( "   Parameter " );
			Serial.print( i );
			Serial.print( " : --");
			Serial.print( argv[i] );
			Serial.println( "--");
		}
	}

	if( pCmd.getError() != 1 )
	{
		Serial.print( "Error:" );
		Serial.println( pCmd.getError());
	}
}

/**
 * Callback for thr command 'test2'.
*/
void CmdTest2( int argc, char *argv[] )
{
	Serial.println( "CmdTest2()");
}

void CmdGetLast( int argc, char *argv[] )
{
	Serial.printf( "CmdGetLast() - %s\n", pCmd.getLastCommand() );
}

void display_freeram() {
  Serial.print(F("- SRAM left: "));
  Serial.println(freeRam());
}

int freeRam() {
  // for AVR (NANO)
  extern int __heap_start,*__brkval;
  int v;
  return (int)&v - (__brkval == 0  
    ? (int)&__heap_start : (int) __brkval);  
}