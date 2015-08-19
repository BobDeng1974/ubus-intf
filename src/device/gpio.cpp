#include <device/gpio.h>

Gpio::Gpio(void)
{
	// nothing for now
}

Gpio::~Gpio(void)
{
	// nothing for now
}


//// public functions
int Gpio::Init(void)
{
	if (!bDebugMode) 	{
		// check that gpio is free
		if ((bRequest = gpio_is_requested(gpioPin)) < 0)
		{
			if (verbosityLevel > 0) printf("gpio_is_requested\n");
			return EXIT_FAILURE;
		}

		// request the pin
		if (!bRequest) {
			if ((gpio_request(gpioPin, NULL)) < 0)
			{
				if (verbosityLevel > 0) printf("gpio_request");
				return EXIT_FAILURE;
			}
		}

		// find if pin is active-low
		if ((bActiveLow = gpio_get_activelow(gpioPin)) < 0)
		{
			if (verbosityLevel > 0) printf("gpio_get_activelow");
			return EXIT_FAILURE;
		}
	}

	return (EXIT_SUCCESS);
}

int Gpio::SetPin(int value, bool bLogicalVaue)
{
	int status 		= EXIT_SUCCESS;

	// adjust for active-low pins
	if (bLogicalVaue) {
		value = bActiveLow ? !value : value;
	}

	// write the value to the pin
	if (!bDebugMode) {
		if ((status = gpio_set_value(gpioPin, value)) < 0) {
			if (verbosityLevel > 0) printf("gpio_set_value");
		}
	}
	else {
		if (verbosityLevel > 0) printf("Setting GPIO ID '%d' to '%d'\n", gpioPin, value);
	}

	return (status);
}

int Gpio::Exit(void)
{
	// release the pin
	if (!bRequest && !bDebugMode) {
		if (gpio_free(gpioPin) < 0)
		{
			if (verbosityLevel > 0) printf("gpio_free");
		}
	}

	return (EXIT_SUCCESS);
}


//// private functions
int Gpio::_Process(char* function)
{
	int status		= EXIT_SUCCESS;


	// find the pin id
	if (JsonGetInt("pin", &gpioPin) < 0)	{
		if (verbosityLevel > 0) printf("invalid_gpio_id\n");
		return EXIT_FAILURE;
	}
	if (verbosityLevel > 0) printf("Using pin '%d'\n", gpioPin);

	// initialize the gpio pin
	Init();


	// check which function is to be used
	if (status == EXIT_SUCCESS)	{
		if ( strcmp(function, GPIO_FUNCTION_SET) == 0 )	{
			status = _FunctionSet();
		}
		else if ( strcmp(function, GPIO_FUNCTION_CLEAR) == 0 )	{
			status = _FunctionClear();
		}
		else if (strcmp(function, GPIO_FUNCTION_SET_PIN) == 0 )	{
			status = _FunctionSetValue();
		}
		else if (strcmp(function, GPIO_FUNCTION_GET) == 0 )	{
			status = _FunctionGet();
		}
		else if (strcmp(function, GPIO_FUNCTION_STATUS) == 0 )	{
			status = _FunctionStatus();
		}
	}


	// free the pin
	Exit();


	return (status);
}

void Gpio::_GenerateJsonOut(int inputStatus)
{
	rapidjson::Value 	element;


	// setup the json object
	jsonOut.SetObject();

	// populate the value
	if (inputStatus == EXIT_SUCCESS)
	{
		element.SetString("true");
	}
	else {
		element.SetString("false");
	}

	// add element to the json object
	jsonOut.AddMember("success", element, jsonOut.GetAllocator() );

	// output the json object
	JsonPrint();
}

// set the pin to logical 1
// 	if active-high:	pin HIGH
// 	if active-low:	pin LOW
int Gpio::_FunctionSet(void)
{
	int status 		= EXIT_SUCCESS;

	// set the pin
	status 	= SetPin(1);

	// generate the json return
	_GenerateJsonOut(status);

	return (status);
}

// set the pin to logical 0
// 	if active-high:	pin LOW
// 	if active-low:	pin HIGH
int Gpio::_FunctionClear(void)
{
	int status 		= 0;

	// set the pin
	status 	= SetPin(0);

	// generate the json return
	_GenerateJsonOut(status);

	return (status);
}

// set the pin to the json value 
// 	if active-high:	0 = pin LOW; 	1 = pin HIGH
// 	if active-low:	0 = pin HIGH; 	1 = pin LOW
int Gpio::_FunctionSetValue(void)
{
	int status 		= EXIT_SUCCESS;
	int value;

	// find the value
	if ((status = JsonGetInt("value", &value)) < 0)	{
		if (verbosityLevel > 0) printf("invalid json int for value\n");
		return EXIT_FAILURE;
	}

	// set the pin
	status 		= SetPin(value);

	// generate the json return
	_GenerateJsonOut(status);

	return (status);
}

int Gpio::_FunctionGet(void)
{
	int status 		= EXIT_SUCCESS;
	int value;

	// read the pin value
	if (!bDebugMode) {
		if ((value = gpio_get_value(gpioPin)) < 0) {
			if (verbosityLevel > 0) printf("gpio_get_value");
			status = EXIT_FAILURE;
		}
	}
	else {
		value = 1;
	}

	// take active-low into account
	//		al 	pin 	value
	//		0	0		0
	//		0	1		1
	//		1	0		1
	//		1	1		0
	// => logical value = al ^ pin
	value 	= (bActiveLow ^ value);

	// generate the output json
	_GenerateGetJson(value);

	return (status);
}

void Gpio::_GenerateGetJson(int logicalValue)
{
	rapidjson::Value 	element;

	// setup the json object
	jsonOut.SetObject();

	//// set the pin number
	// set the element value
	element.SetInt(gpioPin);

	// add element to the json object
	jsonOut.AddMember	(	rapidjson::Value("pin", jsonOut.GetAllocator()).Move(), 
							element, 
							jsonOut.GetAllocator() 
						);

	//// set the pin value
	// set the element value
	element.SetBool((bool)logicalValue);

	// add element to the json object
	jsonOut.AddMember	(	rapidjson::Value("value", jsonOut.GetAllocator()).Move(), 
							element, 
							jsonOut.GetAllocator() 
						);


	// output the json object
	JsonPrint();
}

int Gpio::_FunctionStatus(void)
{
	int status 		= EXIT_SUCCESS;
	int value;

	// set the pin
	status 	= SetPin(value);

	return (status);
}






