#include "src/AccessPermissionManager.h"
#include <SD.h>

AccessPermissionManager permissions;

void setup()
{
	try
	{
		Serial.begin(115200);

		if (!SD.begin())
			throw std::runtime_error("SD card failed to initialize, or not present");

		if (SD.exists("/access_permissions.db"))
			SD.remove("/access_permissions.db");

		permissions.open("/sd/access_permissions.db");
	}
	catch (const std::exception &e)
	{
		Serial.println(e.what());
		while (true)
			;
	}
}

void loop()
{
	try
	{
		permissions.shareFileWithUser("/test.txt", "user1");
		permissions.shareFileWithUser("/test.txt", "user2");
		permissions.shareFileWithUser("/test.txt", "user3");

		Serial.println(permissions.canUserAccessFile("user1", "/test.txt"));
		Serial.println(permissions.canUserAccessFile("user2", "/test.txt"));
		Serial.println(permissions.canUserAccessFile("user3", "/test.txt"));
		Serial.println(permissions.canUserAccessFile("user4", "/test.txt"));
		Serial.println(permissions.canUserAccessFile("user1", "/test2.txt"));

		std::vector<String> users = permissions.getUsersWithAccessToFile("/test.txt");
		for (const auto &user : users)
			Serial.println(user);
	}
	catch (const std::exception &e)
	{
		Serial.println(e.what());
	}
	while (true)
		;
}