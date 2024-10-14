#define NOMINMAX
#include <Windows.h>

#include "Include/Vanilla/AbstractVanillaInterface.hpp"
#include "Include/Vanilla/Components/Structures.hpp"
#include "Include/Vanilla/Components/FlTextControl.hpp"
#include "Include/Vanilla/Components/FlButtonControl.hpp"
#include "Include/Vanilla/Components/FlCycleControl.hpp"
#include "Include/FLUF.UI.hpp"
#include "HudManager.hpp"
#include "Hooks.h"

#pragma comment (lib, "FLUF.UI.lib")

#include <iostream>

FlTextControl* textControl;
FlButtonControl* shipDealerNextControl;
FlButtonControl* shipDealerPrevControl;

FlControl* navBarControl;

std::unordered_map<uint, std::vector<uint>> orderedBaseShipMap;


class ShipDealerInterface : public AbstractVanillaInterface
{
    inline static bool initializedControls;

public:
    ShipDealerInterface()
    {
        initializedControls = false;
    }

private:
    void InitCustomControl()
    {
    }

    // Inherited via AbstractVanillaInterface
    void OnDraw(FlControl* control) override
    {
    }
    void OnUpdate(FlControl* control) override
    {
        if (!initializedControls && control->GetName() == "NN_ShipTrader")
        {
            initializedControls = true;

            if (HasNextShips())
            {
                shipDealerNextControl = this->CreateButtonControl(control, "ShipTraderNextShips", "interface\\NEURONET\\INVENTORY\\inv_sellarrow.3db", nullptr, "ui_select_item", nullptr,
                    { 1.45f, -0.5f, -1.0f },
                    nullptr, { 0.05f, 0.02f, 0.0f }, false, false, false, false, 0, 0, 0);
            }

            if (HasPrevShips())
            {
                shipDealerPrevControl = this->CreateButtonControl(control, "ShipTraderPrevShips", "interface\\NEURONET\\INVENTORY\\inv_buyarrow.3db", nullptr, "ui_select_item", nullptr,
                    { 1.30f, -0.5f, -1.0f }, nullptr, { 0.05f, 0.02f, 0.0f }, false, false, false, false, 0, 0, 0);
            }
        }

        if (!navBarControl && control->GetName() == "NavBar")
        {
            navBarControl = control;
        }
    }

    std::string ToLower(const std::string& scStr)
    {
        std::string scResult;
        for (uint i = 0; (i < scStr.length()); i++)
            scResult += tolower(scStr[i]);

        return scResult;
    }

    FlControl* NavBarFindShipDealer()
    {
        FlControl* child = navBarControl->GetChildControl();
        FlControl* prev = nullptr;
        while (child)
        {
            if (*(uint*)child != 0x5e15f4)
            {
                prev = child;
                child = child->GetNextControl();
                continue;
            }
            FlButtonControl* btn = reinterpret_cast<FlButtonControl*>(child);
            std::string iconStr = ToLower(btn->iconFile);
            if (iconStr == "interface\\baseside\\salesman.3db")
            {
                return child;
            }
            prev = child;
            child = child->GetNextControl();
        }
        return nullptr;
    }

    bool OnExecuteButton(FlButtonControl* control) override
    {
        if (shipDealerNextControl && control->GetName() == "ShipTraderNextShips")
        {
            SetNextShips();
            auto cancelButton = control->GetParentControl()->FindChildControl("ShipTraderCancelButton");
            cancelButton->Execute();

            auto shipDealerBtn = NavBarFindShipDealer();
            if (shipDealerBtn)
            {
                shipDealerBtn->Execute();
            }
        }
        else if (shipDealerPrevControl && control->GetName() == "ShipTraderPrevShips")
        {
            SetPrevShips();
            auto cancelButton = control->GetParentControl()->FindChildControl("ShipTraderCancelButton");
            cancelButton->Execute();

            auto shipDealerBtn = NavBarFindShipDealer();
            if (shipDealerBtn)
            {
                shipDealerBtn->Execute();
            }
        }

        return true;
    }
    bool OnExecuteText(FlTextControl* control) override
    {
        return true;
    }
    void OnExecuteScroll(FlScrollControl* control) override
    {
    }
    void OnExecuteCycle(FlCycleControl* control) override
    {
    }
    void OnTerminate(FlControl* control) override
    {
        auto name = control->GetName();
        if (name == "NN_ShipTrader")
        {
            initializedControls = false;

            shipDealerNextControl = nullptr;
            shipDealerPrevControl = nullptr;
        }
        else if (name == "NavBar")
        {
            navBarControl = nullptr;
        }
    }
};

void InitShipDealerData()
{

    INI_Reader ini;

    char szCurDir[MAX_PATH];
    GetCurrentDirectoryA(sizeof(szCurDir), szCurDir);

    std::string shipMarketFile = std::string(szCurDir) + R"(\..\DATA\EQUIPMENT\market_ships.ini)";
    if (ini.open(shipMarketFile.c_str(), false))
    {
        while (ini.read_header())
        {
            if (!ini.is_header("BaseGood"))
            {
                continue;
            }

            uint currentBase;
            while (ini.read_value())
            {
                if (ini.is_value("base"))
                {
                    currentBase = CreateID(ini.get_value_string());
                }
                else if (ini.is_value("marketgood"))
                {
                    orderedBaseShipMap[currentBase].push_back(CreateID(ini.get_value_string(0)));
                }
            }
        }

        ini.close();
    }
}

void Init()
{
    static ShipDealerInterface hud;
    auto instance = FlufUi::Instance().lock();
    auto lockHudMng = instance->GetHudManager().lock();

    lockHudMng->RegisterHud(&hud);

    InitShipDealerHooks();

    InitShipDealerData();
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Init();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}