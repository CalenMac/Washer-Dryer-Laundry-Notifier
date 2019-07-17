#include <Bridge.h>
#include <Temboo.h>


const int buttonPin = 2;            // pin number of the pushbutton
const int xPin = 3;                 // pin number of the X output of the accelerometer
const int yPin = 4;                 // pin number of the Y output of the accelerometer
const int ledPin = 13;              // pin number of the LED
const int waitTime = 5;             // wait time in minutes
const float sensitivityX = 0.005;   // sensitivity of X axis in percent change
const float sensitivityY = 0.005;   // sensitivity of Y axis in percent change

// Variables:
boolean lastButtonState = LOW;
boolean currentButtonState = LOW;
boolean ledState = LOW;
int counter = 0;
float lastPulseX = 0;
float lastPulseY = 0;

// Initial setup
void setup() {
	// initialize the pins:
	pinMode(xPin, INPUT);
	pinMode(yPin, INPUT);
	pinMode(buttonPin, INPUT);
	pinMode(ledPin, OUTPUT);

	Bridge.begin();
}

// Debounce function for the pushbutton
boolean debounce(boolean last)
{
	boolean current = digitalRead(buttonPin);
	if (last != current)
	{
		delay(5);
		current = digitalRead(buttonPin);
	}
	return current;
}

// Main loop
void loop()
{
	// Set the LED
	currentButtonState = debounce(lastButtonState);
	if (lastButtonState == LOW && currentButtonState == HIGH)
	{
		ledState = !ledState;
	}
	lastButtonState = currentButtonState;
	digitalWrite(ledPin, ledState);

	// If the led is on
	if (ledState == HIGH)
	{
		// If the counter is less than the wait time.
		// I multiplied the waitTime by 3000 to account for timing.
		
		if (counter < (waitTime * 3000))
		{
			// Declare variables to read the pulse widths, change, and percentage change:
			float pulseX, pulseY, changeX, changeY, percentX, percentY;

			// Read pulse from x- and y-axes:
			pulseX = pulseIn(xPin, HIGH);
			pulseY = pulseIn(yPin, HIGH);

			// Find the change in the pulse:
			changeX = lastPulseX - pulseX;
			changeY = lastPulseY - pulseY;

			// Calculate the percentage change using absolute values:
			percentX = abs(changeX / lastPulseX);
			percentY = abs(changeY / lastPulseY);

			// If the percentage change is less than the sensitivity (i.e. no movement detected)
			if (percentX < sensitivityX && percentY < sensitivityY)
			{
				// Increase the counter
				counter++;
			}
			// Else if movement is detected
			else
			{
				// Reset the counter
				counter = 0;
			}

			// Set the last pulse equal to the current pulse
			lastPulseX = pulseX;
			lastPulseY = pulseY;
		}
		else
		{
			// Send an SMS via a Twilio account and Temboo

			// we need a Process object to send a Choreo request to Temboo
			TembooChoreo SendSMSChoreo;

			// invoke the Temboo client
			// NOTE that the client must be reinvoked and repopulated with
			// appropriate arguments each time its run() method is called.
			SendSMSChoreo.begin();

			// set Temboo account credentials
			SendSMSChoreo.setAccountName(TEMBOO_ACCOUNT);
			SendSMSChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
			SendSMSChoreo.setAppKey(TEMBOO_APP_KEY);

			// identify the Temboo Library choreo to run (Twilio > SMSMessages > SendSMS)
			SendSMSChoreo.setChoreo("/Library/Twilio/SMSMessages/SendSMS");

			// set the required choreo inputs
			// see https://www.temboo.com/library/Library/Twilio/SMSMessages/SendSMS/ 
			// for complete details about the inputs for this Choreo

			// the first input is a your AccountSID
			SendSMSChoreo.addInput("AccountSID", TWILIO_ACCOUNT_SID);

			// next is your Auth Token
			SendSMSChoreo.addInput("AuthToken", TWILIO_AUTH_TOKEN);

			// next is your Twilio phone number
			SendSMSChoreo.addInput("From", TWILIO_NUMBER);

			// next, what number to send the SMS to
			SendSMSChoreo.addInput("To", RECIPIENT_NUMBER);

			// finally, the text of the message to send
			SendSMSChoreo.addInput("Body", "Your laundry is ready!");

			// tell the Process to run and wait for the results. The 
			// return code (returnCode) will tell us whether the Temboo client 
			// was able to send our request to the Temboo servers
			unsigned int returnCode = SendSMSChoreo.run();

			SendSMSChoreo.close();

			// Reset the counter
			counter = 0;
		}
	}
}
