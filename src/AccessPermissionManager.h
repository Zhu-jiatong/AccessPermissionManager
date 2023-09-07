#if !defined(_AccessPermissionManager_h)
#define _AccessPermissionManager_h

#include <Arduino.h>

class AccessPermissionManager
{
public:
	AccessPermissionManager() = default;
	~AccessPermissionManager() = default;

private:
	String generateSharingId();
};

#endif // _AccessPermissionManager_h
