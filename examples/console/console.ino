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

		// test begins here
		permissions.shareFileWithUser("/test.txt", "user1", Permission::READ);
		permissions.shareFileWithUser("/testSpecial.txt", "user1", Permission::READ | Permission::WRITE);
		permissions.shareFileWithUser("/test.txt", "user2", Permission::WRITE);
		permissions.shareFileWithUser("/test.txt", "user3", Permission::READ);

		Serial.println(permissions.canUserAccessFile("user1", "/test.txt", Permission::READ | Permission::WRITE));
		Serial.println(permissions.canUserAccessFile("user2", "/test.txt", Permission::READ));
		Serial.println(permissions.canUserAccessFile("user2", "/test.txt", Permission::WRITE));
		Serial.println(permissions.canUserAccessFile("user3", "/test.txt", Permission::READ));
		Serial.println(permissions.canUserAccessFile("user4", "/test.txt", Permission::READ | WRITE));
		Serial.println(permissions.canUserAccessFile("user1", "/test2.txt", Permission::READ | WRITE));
		Serial.println(permissions.canUserAccessFile("user2", "/user2/test2.txt", Permission::WRITE));
		Serial.println(permissions.canUserAccessFile("user2", "/user1/test2.txt", Permission::READ | WRITE));

		std::vector<String> users = permissions.getUsersWithAccessToFile("/test.txt", Permission::READ);
		for (const auto &user : users)
			Serial.println(user);

		std::vector<String> files = permissions.getFilesSharedWithUser("user1", Permission::READ);
		for (const auto &file : files)
			Serial.println(file);
	}
	catch (const std::exception &e)
	{
		Serial.println(e.what());
	}
}

void loop() {}