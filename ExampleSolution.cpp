#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include "IGalaxyPathFinder.h"

#include "json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;

struct Capacity
{
    float half_x;
    float half_y;
    float half_z;
};

struct box
{
	int id;
	int half_x, half_y, half_z;
	int target;
    float weight;
};

struct targetPoint
{
	int id;
	float x, y, z;
    float distance;
};

struct ship
{
    Capacity maxCarryingCapacity;
    float maxVolume;
	float maxCarryingWeight;
    float maxResourcesWeight, currentResourceWeight;
    float resourcesConsumption;
    float base_x, base_y, base_z;
};

struct step
{
    int destinitionPoint;
    std::vector<box> arrayOfBox;
    float resoursesOnShip;
};

class AndriiRudyiPathFinder : public IGalaxyPathFinder
{
    private:
        std::vector<step> _Steps;
	    std::vector<box> _Boxes;
	    std::vector<targetPoint> _TargetPoints;
        std::vector<std::vector<box> > _SortedBoxes;
	    ship _MyShip;
        void ParseJson(const char* inputJasonFile);
        void SortTargetPoints();
        void FindDIstance();
        void SortBoxes();
        bool CanEquipedBox(const box & rhs, const float & BoxVolume, const ship & Ship, const float & CurrentShipVolume, const float & CurrentShipCarryingWeight) const;
        std::vector<box> * FindNeededBoxes(const int & Itsid);
    public:
	    void FindSolution(const char* inputJasonFile, const char* outputFileName) final;
	    const char* ShowCaptainName() final{ return "Andrii Rudyi"; }
};

void AndriiRudyiPathFinder::ParseJson(const char* inputJasonFile)
{
    using std::cout;
    using std::endl;
    try
    {
        std::ifstream i(inputJasonFile);
        if (!i.good())
            throw std::exception();
        json j = json::parse(i, nullptr, false);
        
        // parse ship
        
        this->_MyShip.maxCarryingCapacity.half_x = j["ship"]["maxCarryingCapacity"]["half_x"];
        this->_MyShip.maxCarryingCapacity.half_y = j["ship"]["maxCarryingCapacity"]["half_y"];
        this->_MyShip.maxCarryingCapacity.half_z = j["ship"]["maxCarryingCapacity"]["half_z"];
        this->_MyShip.maxVolume = this->_MyShip.maxCarryingCapacity.half_x * this->_MyShip.maxCarryingCapacity.half_y * this->_MyShip.maxCarryingCapacity.half_z * 8;
        this->_MyShip.maxResourcesWeight = j["ship"]["maxResourcesWeight"];
        this->_MyShip.resourcesConsumption = j["ship"]["resourcesConsumption"];
        this->_MyShip.maxCarryingWeight = j["ship"]["maxCarryingWeight"];
        
        //parse target points
        
        for (auto& x : j["targetPoints"].items())
        {
            targetPoint tmp;
            tmp.id = x.value()["pointId"];
            tmp.x = x.value()["x"];
            tmp.y = x.value()["y"];
            tmp.z = x.value()["z"];
            if (tmp.id == 0)
            {
                _MyShip.base_x = tmp.x;
                _MyShip.base_y = tmp.y;
                _MyShip.base_z = tmp.z;
            }
            _TargetPoints.push_back(tmp);
        }
        
        //parse boxes
        
        for (auto& x : j["boxes"].items())
        {
            box tmp;
            tmp.weight = x.value()["weight"];
            tmp.target = x.value()["targetPointId"];
            tmp.id = x.value()["boxId"];
            tmp.half_x = x.value()["half_x"];
            tmp.half_y = x.value()["half_y"];
            tmp.half_z = x.value()["half_z"];
            _Boxes.push_back(tmp);
        }

    }
    catch(json::exception & e)
    {
        cout << "json error message: " << e.what() << endl;
    }
    catch(std::exception &e)
    {
        cout << "std::exception error message: " << e.what() << endl;
    }
    catch(...)
    {
        cout << "Something bad happened\n";
    }
}

void AndriiRudyiPathFinder::FindDIstance()//find distance from base to targets
{
    for (auto it = _TargetPoints.begin(); it != _TargetPoints.end(); ++it)
    {
        if ( it->id != 0)
        {
            it->distance = sqrt( pow(it->x - _MyShip.base_x, 2) + pow(it->y - _MyShip.base_y, 2) + pow(it->z - _MyShip.base_z, 2)) ;
        }
        else
        {
            it->distance = 0;
        }
    }
}

void AndriiRudyiPathFinder::SortTargetPoints()
{
    std::sort(_TargetPoints.begin(), _TargetPoints.end(), [](targetPoint const& f, targetPoint const& s){ return f.distance < s.distance; });
}

void AndriiRudyiPathFinder::SortBoxes()
{
    bool is_pushed;
    for (auto it1 = _Boxes.begin() ; it1 != _Boxes.end(); ++it1)
    {
        is_pushed = false;
        for (auto it2 = _SortedBoxes.begin() ; it2 != _SortedBoxes.end(); it2++)
        {
            if (it1->target == it2->begin()->target)
            {
                it2->push_back(*it1);
                is_pushed = true;
                break ;
            }
        }
        if (is_pushed == false)
        {
            _SortedBoxes.push_back( std::vector<box>());
            _SortedBoxes.back().push_back(*it1);
        }
    }
    _Boxes.clear();
    for (auto it1 = _SortedBoxes.begin() ; it1 != _SortedBoxes.end(); ++it1)
    {
        std::sort(it1->begin(), it1->end(), [](box const& f, box const& s){ return f.weight < s.weight; });
        /* for(auto &x : *it1)
        {
            std::cout << x.id << std::endl;
        }
        //std::cout << it1->size() << std::endl;
        break ; */
    }
}

bool AndriiRudyiPathFinder::CanEquipedBox(const box & rhs, const float & BoxVolume, const ship & Ship, const float & CurrentShipVolume, const float & CurrentShipCarryingWeight) const
{
    using std::cout;
    using std::endl;

    if (rhs.half_x > _MyShip.maxCarryingCapacity.half_x || rhs.half_y > _MyShip.maxCarryingCapacity.half_y || rhs.half_z > _MyShip.maxCarryingCapacity.half_z)
    {
        cout << "half_X" << endl;
        return false;
    }
    else if ((rhs.weight + CurrentShipCarryingWeight) > Ship.maxCarryingWeight)
    {
        cout << "weight" << endl;
        return false;
    }
    else if ((BoxVolume + CurrentShipVolume) > Ship.maxVolume)
    {
        cout << "volume" << endl;
        return false;
    }
    return true;
}

std::vector<box> * AndriiRudyiPathFinder::FindNeededBoxes(const int & Itsid)
{
    for (auto it = _SortedBoxes.begin(); it != _SortedBoxes.end(); ++it)
    {
        if (it->front().target == Itsid)
        {
            return &(*it);
        }
    }
    return nullptr;
}

void AndriiRudyiPathFinder::FindSolution(const char* inputJasonFile, const char* outputFileName)
{
    using std::cout;
    using std::endl;
    std::vector <step> tmpStep;
    std::vector<box> * tmpBox;
    float BoxVolume;
    float CurrentShipVolume;
    float CurrentResourses;
    float CurrentShipCarryingWeight;
    bool trynext = false;

    this->ParseJson(inputJasonFile);
    this->FindDIstance();
    this->SortTargetPoints();
	this->SortBoxes();

    //preparing algorithm
    tmpStep.push_back(step());
    tmpStep.back().arrayOfBox = std::vector<box>();
    tmpStep.back().arrayOfBox.push_back(box());
    CurrentShipVolume = 0;

    for ( auto TargetPtr = _TargetPoints.begin() + 1; TargetPtr != _TargetPoints.end(); ++TargetPtr)
    {
        if (trynext == false)
        {
            CurrentResourses = TargetPtr->distance * _MyShip.resourcesConsumption * 2;
            CurrentShipCarryingWeight = CurrentResourses;
        }
        else
        {
            //CurrentResourses += ;
            //CurrentShipCarryingWeight += CurrentResourses;
        }
        if ((tmpBox = this->FindNeededBoxes(TargetPtr->id)) == nullptr) // find needed boxes
        {
            cout << "nullptr\n";
            continue ;
        }
        for (auto BoxPtr = tmpBox->begin(); BoxPtr != tmpBox->end(); ++BoxPtr )
        {
            BoxVolume = (BoxPtr->half_x * BoxPtr->half_y * BoxPtr->half_z * 8);
            
            if (this->CanEquipedBox(*BoxPtr, BoxVolume, _MyShip, CurrentShipVolume, CurrentShipCarryingWeight) == true)
            {
                CurrentShipVolume += BoxVolume;
                CurrentShipCarryingWeight += BoxPtr->weight;
                tmpStep.back().destinitionPoint = TargetPtr->id;
                tmpStep.back().resoursesOnShip = CurrentResourses;
                tmpStep.back().arrayOfBox.back() = *BoxPtr;
                
                //cout << "DestPoint: " << tmpStep.back().destinitionPoint << " CurrentShipCarryingWeight: " << CurrentShipCarryingWeight << " IdBox: " << tmpStep.back().arrayOfBox.back().id << endl;              
                
                tmpStep.back().arrayOfBox.push_back(box());
                tmpBox->erase(BoxPtr);//delete box from vector // inefficient !!!
                BoxPtr--;
                if (BoxPtr + 1 == tmpBox->end())
                    trynext = true;
            }
            else
            {
                // it's tempory
                tmpStep.push_back(step());
                tmpStep.back().arrayOfBox = std::vector<box>();
                tmpStep.back().arrayOfBox.push_back(box());
                
                tmpStep.back().destinitionPoint = 0;
                tmpStep.back().resoursesOnShip = 0.0;
                tmpStep.back().arrayOfBox.back() = box();
                
                tmpStep.push_back(step());
                tmpStep.back().arrayOfBox = std::vector<box>();
                tmpStep.back().arrayOfBox.push_back(box());
                // it's tempory

                //cout << "Not equiped\n";
                trynext = false;
                break ;
            }
        }
    }
    
    json j, d;
    j["steps"] = json::array();
    d = json::array();
    for (auto it1 = tmpStep.begin(); it1 != tmpStep.end(); it1++)
    {
        for (auto it2 = it1->arrayOfBox.begin(); it2 != it1->arrayOfBox.end(); it2++)
            d.push_back(json::object( { {"boxId", it2->id}, {"x", it2->half_x}, {"y", it2->half_y}, {"z", it2->half_z} } ) ) ;
    
        j["steps"].push_back(json::object( { {"shippedBoxes", d }, {"destinationPointId", it1->destinitionPoint}, {"shippedResources", it1->resoursesOnShip} }   ) );
    }
    std::ofstream o(outputFileName);
	o << std::setw(4) << j << std::endl;
}

//cout << "Target: " << BoxPtr->target << " id: " << BoxPtr->id << " Volume: "<< BoxVolume << endl;
//cout << "DestPoint: " << tmpStep.back().destinitionPoint << " ResOnShip: " << tmpStep.back().resoursesOnShip << " IdBox: " << tmpStep.back().arrayOfBox.back().id << endl;
                

