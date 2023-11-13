#include "AccessPermissionManager.h"
#include <mbedtls/sha256.h>
#include <sstream>
#include <iomanip>
#include <FilesystemUtility.h>

void AccessPermissionManager::open(const String &sharingRegistryPath)
{
	_sharingRegistry.open(sharingRegistryPath);
	_sharingRegistry.execute("CREATE TABLE IF NOT EXISTS sharing_registry ("
							 "username TEXT NOT NULL,"
							 "path TEXT NOT NULL,"
							 "permissions_flags INTEGER NOT NULL)");
}

bool AccessPermissionManager::canUserAccessFile(const String &username, const String &path, FilePermissions permissions)
{
	// user owns the file
	if (FSUtil::isChildOf("/" + username, path))
		return true;

	// user is shared the file
	JSONVar result = _sharingRegistry.execute("SELECT 1 "
											  "FROM sharing_registry "
											  "WHERE username = ? AND path = ? AND permissions_flags & :flags = :flags",
											  username,
											  path,
											  static_cast<int>(permissions.to_ulong()));
	if (result.length() == -1)
		return false;
	return true;
}

void AccessPermissionManager::shareFileWithUser(const String &path, const String &username, FilePermissions permissions)
{
	_sharingRegistry.execute("INSERT INTO sharing_registry "
							 "VALUES (:username, :path, :permissions_flags)",
							 username,
							 path,
							 static_cast<int>(permissions.to_ulong()));
}

void AccessPermissionManager::unshareFileWithUser(const String &path, const String &username)
{
	_sharingRegistry.execute("DELETE FROM sharing_registry "
							 "WHERE username = ? AND path = ?",
							 username,
							 path);
}

void AccessPermissionManager::unshareFile(const String &path)
{
	_sharingRegistry.execute("DELETE FROM sharing_registry "
							 "WHERE path = ?",
							 path);
}

std::vector<String> AccessPermissionManager::getUsersWithAccessToFile(const String &path, FilePermissions permissions)
{
	JSONVar result = _sharingRegistry.execute("SELECT username "
											  "FROM sharing_registry "
											  "WHERE path = ? AND permissions_flags & :flags = :flags",
											  path,
											  static_cast<int>(permissions.to_ulong()));
	if (result.length() == -1)
		return std::vector<String>();

	std::vector<String> users;
	for (int i = 0; i < result.length(); i++)
		users.push_back(result[i]["username"]);
	return users;
}

std::vector<String> AccessPermissionManager::getFilesSharedWithUser(const String &username, FilePermissions permissions)
{
	JSONVar result = _sharingRegistry.execute("SELECT path "
											  "FROM sharing_registry "
											  "WHERE username = ? AND permissions_flags & :flags = :flags",
											  username,
											  static_cast<int>(permissions.to_ulong()));

	if (result.length() == -1)
		return std::vector<String>();

	std::vector<String> files;
	for (int i = 0; i < result.length(); i++)
		files.push_back(result[i]["path"]);
	return files;
}

String AccessPermissionManager::generateSharingId(const String &user, const String &path)
{
	uint8_t sharingId[32];
	String input = time(0) + path + esp_random();

	int result = mbedtls_sha256_ret(
		reinterpret_cast<const uint8_t *>(input.c_str()),
		input.length(),
		sharingId,
		0);

	if (result)
		throw std::runtime_error("cannot generate sharing ID");

	std::stringstream resultStringBuilder;
	for (auto &&i : sharingId)
		resultStringBuilder << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(i);

	return resultStringBuilder.str().c_str();
}