#include <windows.h>
#include "HudManager.hpp"
#include "Include/flmap.h"
#include <vector>
#include "Hooks.h"

PBYTE renderShipOrgData;
typedef bool(__thiscall* renderShipposFuncOrg)(void* ShipDealer, int a1, int a2);
renderShipposFuncOrg renderShipposFunc = renderShipposFuncOrg(0x4B7440);

struct ShipListEntry
{
	ShipListEntry* next;
	ShipListEntry* prev;
	uint* shipArchId;
};

struct ShipList
{
	uint allocator;
	ShipListEntry* firstEntry;
	uint shipCounter;
};

static std::vector<uint*> shipsToShow;
static FlMap<uint, MarketGoodInfo>* mappy;

void DetourFirst(void* pOFunc, void* pHkFunc, unsigned char* originalData)
{
	DWORD dwOldProtection = 0; // Create a DWORD for VirtualProtect calls to allow us to write.
	BYTE bPatch[5]; // We need to change 5 bytes and I'm going to use memcpy so this is the simplest way.
	bPatch[0] = 0xE9; // Set the first byte of the byte array to the op code for the JMP instruction.
	VirtualProtect(pOFunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtection); // Allow us to write to the memory we need to change
	DWORD dwRelativeAddress = (DWORD)pHkFunc - (DWORD)pOFunc - 5; // Calculate the relative JMP address.
	memcpy(&bPatch[1], &dwRelativeAddress, 4); // Copy the relative address to the byte array.
	memcpy(originalData, pOFunc, 5);
	memcpy(pOFunc, bPatch, 5); // Change the first 5 bytes to the JMP instruction.
}

void Detour(void* pOFunc, void* pHkFunc, unsigned char* originalData)
{
	BYTE bPatch[5]; // We need to change 5 bytes and I'm going to use memcpy so this is the simplest way.
	bPatch[0] = 0xE9; // Set the first byte of the byte array to the op code for the JMP instruction.
	DWORD dwRelativeAddress = (DWORD)pHkFunc - (DWORD)pOFunc - 5; // Calculate the relative JMP address.
	memcpy(&bPatch[1], &dwRelativeAddress, 4); // Copy the relative address to the byte array.
	memcpy(originalData, pOFunc, 5);
	memcpy(pOFunc, bPatch, 5); // Change the first 5 bytes to the JMP instruction.
}

void UnDetour(void* pOFunc, unsigned char* originalData)
{
	memcpy(pOFunc, originalData, 5);
}

int __fastcall GetShipListDetour(void* baseInfo, void* edx, ShipList* sf, int searchedType)
{
	for (uint* shipId : shipsToShow)
	{
		ShipListEntry* firstNode = sf->firstEntry;
		ShipListEntry* lastNode = firstNode->prev;

		ShipListEntry* newNode = (ShipListEntry*)malloc(0xC);
		newNode->next = firstNode; // new element next = start
		newNode->prev = lastNode;

		lastNode->next = newNode;
		firstNode->prev = newNode;

		newNode->shipArchId = shipId;
		sf->shipCounter++;
	}
	return sf->shipCounter;
}

const static uint* playerBase = (uint*)0x66873C;
static void* lastShipDealer = nullptr;
static int currShipPage = 0;
static int currBaseShipCount;

void ReloadShipCount()
{
	static uint lastBase = 0;

	currBaseShipCount = 0;

	BaseData* base_data = BaseDataList_get()->get_base_data(*playerBase);
	mappy = reinterpret_cast<FlMap<uint, MarketGoodInfo>*>(&base_data->market_map);

	for (auto marketGood = mappy->begin(); marketGood != mappy->end() && marketGood.key() != 0; marketGood.Inc())
	{
		uint type = GoodList::find_by_id(marketGood.value()->iGoodID)->iType;
		if (type != GOODINFO_TYPE_SHIP)
		{
			continue;
		}
		currBaseShipCount++;
	}
}

bool CheckShipOverflow()
{
	ReloadShipCount();
	if (currShipPage > (currBaseShipCount - 1) / 3)
	{
		currShipPage = 0;
		return false;
	}
	return true;
}

bool HasPrevShips()
{
	CheckShipOverflow();
	if (currBaseShipCount > 3)
	{
		return true;
	}

	return false;
}

bool HasNextShips()
{
	CheckShipOverflow();
	if (currBaseShipCount > 3)
	{
		return true;
	}

	return false;
}

void SetNextShips()
{
	if (CheckShipOverflow())
	{
		currShipPage++;
	}
}

void SetPrevShips()
{
	if (currShipPage)
	{
		currShipPage--;
	}
	else
	{
		currShipPage = (currBaseShipCount - 1) / 3;
	}
}

bool GetOrderedShips(int page)
{
	auto baseShipIter = orderedBaseShipMap.find(*playerBase);
	if (baseShipIter != orderedBaseShipMap.end() && !baseShipIter->second.empty())
	{
		shipsToShow.clear();
		int lastItem = min((page+1) * 3, baseShipIter->second.size());

		for (int i = page * 3; i < lastItem; i++)
		{
			shipsToShow.push_back(&baseShipIter->second.at(i));
		}

		return true;
	}

	return false;
}

bool __fastcall RenderShippos(void* ShipDealer, void* edx, int a1, int a2)
{
	static int lastBase = *playerBase;

	if (lastBase != *playerBase)
	{
		currShipPage = 0;
		lastBase = *playerBase;
	}

	std::vector<uint*> firstThreeShips;
	if (!GetOrderedShips(currShipPage))
	{
		int counter = 0;

		lastShipDealer = ShipDealer;

		BaseData* base_data = BaseDataList_get()->get_base_data(*playerBase);
		mappy = reinterpret_cast<FlMap<uint, MarketGoodInfo>*>(&base_data->market_map);

		shipsToShow.clear();

		for (auto marketGood = mappy->begin(); marketGood != mappy->end() && marketGood.key() != 0; marketGood.Inc())
		{
			uint type = GoodList::find_by_id(marketGood.value()->iGoodID)->iType;
			if (type != GOODINFO_TYPE_SHIP)
			{
				continue;
			}

			if (counter < 3)
			{
				firstThreeShips.push_back(&marketGood.value()->iGoodID);
			}

			if (counter >= currShipPage * 3 && counter <= (currShipPage * 3) + 2)
			{
				shipsToShow.push_back(&marketGood.value()->iGoodID);
			}
			counter++;
		}
	}

	if (shipsToShow.empty())
	{
		currShipPage = 0;
		shipsToShow = firstThreeShips;
	}

	UnDetour(renderShipposFunc, renderShipOrgData);
	bool retVal = renderShipposFunc(ShipDealer, a1, a2);
	Detour(renderShipposFunc, RenderShippos, renderShipOrgData);

	return retVal;
}

void __cdecl FreeShipData(void* shipData)
{
	free(shipData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteProcMem(void* pAddress, void* pMem, int iSize)
{
	HANDLE hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	DWORD dwOld;
	VirtualProtectEx(hProc, pAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOld);
	WriteProcessMemory(hProc, pAddress, pMem, iSize, 0);
	CloseHandle(hProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ReadProcMem(void* pAddress, void* pMem, int iSize)
{
	HANDLE hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	DWORD dwOld;
	VirtualProtectEx(hProc, pAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOld);
	ReadProcessMemory(hProc, pAddress, pMem, iSize, 0);
	CloseHandle(hProc);
}

FARPROC PatchCallAddr(char* hMod, DWORD dwInstallAddress, char* dwHookFunction)
{
	DWORD dwRelAddr;
	ReadProcMem(hMod + dwInstallAddress + 1, &dwRelAddr, 4);

	DWORD dwOffset = (DWORD)dwHookFunction - (DWORD)(hMod + dwInstallAddress + 5);
	WriteProcMem(hMod + dwInstallAddress + 1, &dwOffset, 4);

	return (FARPROC)(hMod + dwRelAddr + dwInstallAddress + 5);
}

void InitShipDealerHooks()
{
	HANDLE hFreelancer = GetModuleHandle(0);
	renderShipOrgData = PBYTE(malloc(5));
	DetourFirst(renderShipposFunc, RenderShippos, renderShipOrgData);
	PatchCallAddr((char*)hFreelancer, 0xB74AC, (char*)GetShipListDetour);
	PatchCallAddr((char*)hFreelancer, 0x77C4B, (char*)GetShipListDetour);
	PatchCallAddr((char*)hFreelancer, 0xB85AA, (char*)FreeShipData);
	PatchCallAddr((char*)hFreelancer, 0x77CB4, (char*)FreeShipData);
}