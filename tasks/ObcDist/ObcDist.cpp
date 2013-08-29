/*
    Copyright (c) 2013 <benemorius@gmail.com>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ObcDist.h"
#include <ObcUI.h>
#include <cstdlib>

using namespace ObcDistState;

ObcDist::ObcDist(OpenOBC& obc) : ObcUITask(obc)
{
	setDisplay("ObcDist");
	state = Run;
	initialDistanceToTravelKm = distanceToTravelKm = strtof(obc.config->getValueByNameWithDefault("ObcDistDistanceKm", "%f", 0).c_str(), NULL);
}

ObcDist::~ObcDist()
{

}

void ObcDist::wake()
{
	odometerStartKm = obc.currentKm;
	runTask();
}

void ObcDist::sleep()
{
	obc.config->setValueByName("ObcDistDistanceKm", "%f", distanceToTravelKm);
}

void ObcDist::runTask()
{
	distanceTraveledKm = obc.currentKm - odometerStartKm;
	distanceToTravelKm = initialDistanceToTravelKm - distanceTraveledKm;
	
	if(state == Run)
	{
		float currentSpeedKmh = obc.averageKmh;
		if(currentSpeedKmh < 1)
			currentSpeedKmh = 100;
		uint32_t seconds = distanceToTravelKm / (currentSpeedKmh / 60 / 60);
		uint32_t minutes = seconds / 60;
		uint32_t hours = minutes / 60;
		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Both)
		{
			setDisplay("% 4ukm % 4umls % 2u:%02u", (uint32_t)distanceToTravelKm, (uint32_t)(distanceToTravelKm * 0.621371), hours, minutes % 60);
		}
		else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Metric)
		{
			setDisplay("dist % 4u km % 2u:%02u", (uint32_t)distanceToTravelKm, hours, minutes % 60);
		}
		else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
		{
			setDisplay("dist % 4u mls % 2u:%02u", (uint32_t)(distanceToTravelKm * 0.621371), hours, minutes % 60);
		}
	}
	else if(state == Set)
	{
		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Both)
		{
			setDisplay("set % 4u km % 4u mls", (uint32_t)distanceKmSet, (uint32_t)(distanceKmSet * 0.621371));
		}
		else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Metric)
		{
			setDisplay("set % 4u km", (uint32_t)distanceKmSet);
		}
		else if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
		{
			setDisplay("set % 4u miles", (uint32_t)(distanceKmSet * 0.621371));
		}
	}
}

void ObcDist::buttonHandler(ObcUITaskFocus::type focus, uint32_t buttonMask)
{
	if(focus == ObcUITaskFocus::background)
	{
		if(buttonMask == BUTTON_DIST_MASK)
			obc.ui->setActiveTask(this);
		return;
	}
	
	if(buttonMask == BUTTON_DIST_MASK)
	{
		if(state == Set)
		{
			initialDistanceToTravelKm = distanceToTravelKm = 0;
			state = Run;
		}
	}
	
	if(buttonMask == BUTTON_SET_MASK)
	{
		if(state == Run)
		{
			state = Set;
			distanceKmSet = distanceToTravelKm;
			if(distanceKmSet < 0)
				distanceKmSet = 0;
		}
		else if(state == Set)
		{
			state = Run;
			initialDistanceToTravelKm = distanceToTravelKm = distanceKmSet;
			odometerStartKm = obc.currentKm;
		}
	}
	
	//TODO fix very bad handling of miles and overflows
	if((state == Set) && (buttonMask & (BUTTON_1000_MASK | BUTTON_100_MASK | BUTTON_10_MASK | BUTTON_1_MASK | BUTTON_SET_MASK)))
	{
		if(buttonMask == BUTTON_1000_MASK)
		{
			if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
				distanceKmSet += 1000 / 0.621371;
			else
				distanceKmSet += 1000;
		}
		if(buttonMask == BUTTON_100_MASK)
		{
			if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
				distanceKmSet += 100 / 0.621371;
			else
				distanceKmSet += 100;
		}
		if(buttonMask == BUTTON_10_MASK)
		{
			if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
				distanceKmSet += 10 / 0.621371;
			else
				distanceKmSet += 10;
		}
		if(buttonMask == BUTTON_1_MASK)
		{
			if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
				distanceKmSet += 1 / 0.621371;
			else
				distanceKmSet += 1;
		}
// 		if(obc.ui->getMeasurementSystem() == ObcUIMeasurementSystem::Imperial)
// 			distanceKmSet = (int32_t)distanceKmSet % (uint32_t)(10000 / 0.621371);
// 		else
// 			distanceKmSet = (int32_t)distanceKmSet % 10000;
		
	}
}
