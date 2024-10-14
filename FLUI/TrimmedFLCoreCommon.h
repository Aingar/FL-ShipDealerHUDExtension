#pragma once

#define IMPORT __declspec(dllimport)
#define EXPORT __declspec(dllexport)
#define OBJECT_DATA_SIZE 2048

typedef unsigned int uint;

#include <map>
#include <list>
#include <unordered_map>

#pragma comment( lib, "FLCoreCommon.lib" )

IMPORT bool  GetUserDataPath(char* const);
IMPORT unsigned int  CreateID(char const*);

class IMPORT BaseDataList
{
public:
	BaseDataList(class BaseDataList const&);
	BaseDataList(void);
	~BaseDataList(void);
	class BaseDataList& operator=(class BaseDataList const&);
	class BaseData* get_base_data(unsigned int)const;
	std::list<class BaseData*>* get_base_data_list(void);
	std::list<class BaseData*> const* get_const_base_data_list(void)const;
	class RoomData* get_room_data(unsigned int)const;
	class RoomData* get_unloaded_room_data(unsigned int)const;
	void load(void);
	void load_market_data(char const*);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

IMPORT class BaseDataList* BaseDataList_get(void);

struct IMPORT CacheString
{
	void clear(void);

public:
	char* value;
};
struct IMPORT EquipDesc
{
	EquipDesc(struct EquipDesc const&);
	EquipDesc(void);
	struct EquipDesc& operator=(struct EquipDesc const&);
	bool operator==(struct EquipDesc const&)const;
	bool operator!=(struct EquipDesc const&)const;
	bool operator<(struct EquipDesc const&)const;
	bool operator>(struct EquipDesc const&)const;

	static struct CacheString const  CARGO_BAY_HP_NAME;
	unsigned int get_arch_id(void)const;
	float get_cargo_space_occupied(void)const;
	int get_count(void)const;
	struct CacheString const& get_hardpoint(void)const;
	unsigned short get_id(void)const;
	int get_owner(void)const;
	float get_status(void)const;
	bool get_temporary(void)const;
	bool is_equipped(void)const;
	bool is_internal(void)const;
	void make_internal(void);
	void set_arch_id(unsigned int);
	void set_count(int);
	void set_equipped(bool);
	void set_hardpoint(struct CacheString const&);
	void set_id(unsigned short);
	void set_owner(int);
	void set_status(float);
	void set_temporary(bool);

public:
	USHORT iDunno;
	USHORT sID;
	UINT iArchID;
	CacheString szHardPoint;
	bool bMounted;
	float fHealth;
	UINT iCount;
	bool bMission;
	uint iOwner;
};

class IMPORT EquipDescList
{
public:
	EquipDescList(struct EquipDescVector const&);
	EquipDescList(class EquipDescList const&);
	EquipDescList(void);
	~EquipDescList(void);
	class EquipDescList& operator=(class EquipDescList const&);
	int add_equipment_item(struct EquipDesc const&, bool);
	void append(class EquipDescList const&);
	struct EquipDesc* find_equipment_item(struct CacheString const&);
	struct EquipDesc* find_equipment_item(unsigned short);
	struct EquipDesc const* find_equipment_item(struct CacheString const&)const;
	struct EquipDesc const* find_equipment_item(unsigned short)const;
	struct EquipDesc const* find_matching_cargo(unsigned int, int, float)const;
	float get_cargo_space_occupied(void)const;
	int remove_equipment_item(unsigned short, int);
	struct EquipDesc* traverse_equipment_type(unsigned int, struct EquipDesc const*);
	struct EquipDesc const* traverse_equipment_type(unsigned int, struct EquipDesc const*)const;

public:
	uint iDunno;
	std::list<EquipDesc> equip;
};

struct GoodInfo
{
public:
#define GOODINFO_TYPE_COMMODITY 0
#define GOODINFO_TYPE_EQUIPMENT 1
#define GOODINFO_TYPE_HULL 2
#define GOODINFO_TYPE_SHIP 3

	uint i1;
	uint iLen;
	uint iDunno1[16];
	/* 72 */ uint iArchID;
	/* 76 */ uint iType; // 0=commodity, 2=hull, 3=ship
	/* 80 */ uint i3;
	/* 84 */ uint iShipGoodID; // if type = GOODINFO_TYPE_HULL
	/* 88 */ float fPrice;
	/* 92 */ float fGoodSellPrice;
	/* 96 */ float fBadBuyPrice;
	/* 100 */ float fBadSellPrice;
	/* 104 */ float fGoodBuyPrice;
	/* 108 */ uint iJumpDist;
	/* 112 */ float iDunno2;
	/* 116 */ float iDunno3;
	/* 120 */ float iDunno4;
	/* 124 */ float iDunno5;
	/* 128 */ float iDunno6;
	/* 132 */ float iDunno7;
	/* 136 */ uint iIDSName;
	/* 140 */ uint iIDS;
	/* 144 */ uint iHullGoodID; // if type = GOODINFO_TYPE_SHIP
	/* 148 */ EquipDescList edl;
	/* 160 */ EquipDescList edl2;
	/* 172 */ EquipDescList edl3;
	/* 184 */ uint iFreeAmmoArchID;
	/* 188 */ uint iFreeAmmoCount;
};

namespace GoodList
{
	IMPORT  struct GoodInfo const* find_by_archetype(unsigned int);
	IMPORT  struct GoodInfo const* find_by_id(unsigned int);
	IMPORT  struct GoodInfo const* find_by_nickname(char const*);
};
enum TransactionType
{
	TransactionType_Sell = 0,
	TransactionType_Buy = 1
};

struct IMPORT MarketGoodInfo
{
	MarketGoodInfo(void);
	struct MarketGoodInfo& operator=(struct MarketGoodInfo const&);

public:
	uint iGoodID;
	float fPrice;
	int iMin;
	int iStock;
	TransactionType iTransType;
	float fRank;
	float fRep;
	int iQuantity;
	float fScale;
};

class IMPORT BaseData
{
public:
	BaseData(class BaseData const&);
	BaseData(void);
	~BaseData(void);
	class BaseData& operator=(class BaseData const&);
	unsigned int get_base_id(void)const;
	std::list<class RoomData*> const* get_const_room_data_list(void)const;
	std::map<unsigned int, struct MarketGoodInfo, struct std::less<unsigned int>, class std::allocator<struct MarketGoodInfo>> const* get_market(void)const;
	float get_price_variance(void)const;
	std::list<class RoomData*>* get_room_data_list(void);
	float get_ship_repair_cost(void)const;
	unsigned int get_start_location(void)const;
	void read_from_ini(char const*, unsigned int);
	void set_market_good(unsigned int, int, int, enum TransactionType, float, float, float);

private:
	void read_Base_block(class INI_Reader*);
	void read_Room_block(class INI_Reader*);

public:
	uint baseId;
	uint start_room;
	float price_variance;
	float ship_repair_cost;
	std::map<uint, MarketGoodInfo> market_map;
};


class IMPORT INI_Reader
{
public:
	INI_Reader(class INI_Reader const&);
	INI_Reader(void);
	~INI_Reader(void);
	class INI_Reader& operator=(class INI_Reader const&);
	void close(void);
	bool find_header(char const*);
	bool get_bool(unsigned int);
	char const* get_file_name(void)const;
	char const* get_header_ptr(void);
	char const* get_indexed_value(unsigned int);
	int get_line_num(void)const;
	char const* get_line_ptr(void);
	char const* get_name(void)const;
	char const* get_name_ptr(void);
	unsigned int get_num_parameters(void)const;
	void get_state(struct State&)const;
	bool get_value_bool(unsigned int);
	float get_value_float(unsigned int);
	int get_value_int(unsigned int);
	char const* get_value_ptr(void);
	char const* get_value_string(unsigned int);
	char const* get_value_string(void);
	unsigned short const* get_value_wstring(void);
	class Vector  get_vector(void);
	bool is_end(void)const;
	bool is_header(char const*);
	bool is_number(char const*)const;
	bool is_value(char const*);
	bool is_value_empty(unsigned int);
	void log_link(char const*)const;
	bool open(char const*, bool);
	bool open_memory(char const*, unsigned int);
	bool read_header(void);
	bool read_value(void);
	void reset(void);
	void seek(unsigned int);
	void set_state(struct State const&);
	unsigned int tell(void)const;
	double value_num(unsigned int);

public:
	unsigned char data[5480];
};