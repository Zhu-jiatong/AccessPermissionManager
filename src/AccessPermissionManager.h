#if !defined(_AccessPermissionManager_h)
#define _AccessPermissionManager_h

#include <Arduino.h>
#include <vector>
#include "SQLiteDatabaseManager/src/DatabaseConnection.h"

class AccessPermissionManager
{
public:
	AccessPermissionManager() = default;
	~AccessPermissionManager() = default;

	void open(const String &sharingRegistryPath);
	bool canUserAccessFile(const String &username, const String &path);
	void shareFileWithUser(const String &path, const String &username);
	void unshareFileWithUser(const String &path, const String &username);
	void unshareFile(const String &path);
	std::vector<String> getUsersWithAccessToFile(const String &path);
	std::vector<String> getFilesSharedWithUser(const String &username);

private:
	DatabaseConnection _sharingRegistry;

	String generateSharingId(const String &user, const String &path);
};

#endif // _AccessPermissionManager_h
