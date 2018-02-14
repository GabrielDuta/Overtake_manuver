#ifndef VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_
#define VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_

#include <list>
#include <string>
#include <stdint.h>

#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/base/utils/Coord.h"

#include "veins/modules/application/platooning/CC_Const.h"

namespace Veins {

class TraCIConnection;

class TraCICommandInterface
{
	public:
		TraCICommandInterface(TraCIConnection&);

		enum DepartDefs {
			DEPART_NOW = 2,
			DEPART_LANE_BEST_FREE = 5,
			DEPART_POS_BASE = 4,
			DEPART_SPEED_MAX = 3
		};

		// General methods that do not deal with a particular object in the simulation
		std::pair<uint32_t, std::string> getVersion();
		std::pair<double, double> getLonLat(const Coord&);
		double getDistance(const Coord& position1, const Coord& position2, bool returnDrivingDistance);

		// Vehicle methods
		bool addVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st = -DEPART_NOW, double emitPosition = -DEPART_POS_BASE, double emitSpeed = -DEPART_SPEED_MAX, int8_t emitLane = -DEPART_LANE_BEST_FREE);
		void executePlexeTimestep();
		class Vehicle {
			public:
				Vehicle(TraCICommandInterface* traci, std::string nodeId) : traci(traci), nodeId(nodeId) {
					connection = &traci->connection;
				}

				void setSpeedMode(int32_t bitset);
				void setSpeed(double speed);
				void setColor(const TraCIColor& color);
				void slowDown(double speed, int time);
				void newRoute(std::string roadId);
				void setParking();
				std::string getRoadId();
				std::string getCurrentRoadOnRoute();
				std::string getLaneId();
				double getLanePosition();
				std::list<std::string> getPlannedRoadIds();
				std::string getRouteId();
				void changeRoute(std::string roadId, double travelTime);
				void stopAt(std::string roadId, double pos, uint8_t laneid, double radius, double waittime);
				int32_t getLaneIndex();
				std::string getTypeId();
				bool changeVehicleRoute(const std::list<std::string>& roads);
				void setLaneChangeMode(int mode);
				void getLaneChangeState(int direction, int &state1, int &state2);
				void changeLane(int lane, int duration);
				double getLength();
				void setParameter(const std::string &parameter, int value);
				void setParameter(const std::string &parameter, double value);
				void setParameter(const std::string &parameter, const std::string &value);
				void getParameter(const std::string &parameter, int &value);
				void getParameter(const std::string &parameter, double &value);
				void getParameter(const std::string &parameter, std::string &value);
				/**
				 * Gets the total number of lanes on the edge the vehicle is currently traveling
				 */
				unsigned int getLanesCount();
				/**
				 * Sets the data about the leader of the platoon. This data is usually received
				 * by means of wireless communications
				 */
				void setLeaderVehicleData(double controllerAcceleration, double acceleration, double speed, double positionX, double positionY, double time);
				void setPlatoonLeaderData(double leaderSpeed, double leaderAcceleration, double positionX, double positionY, double time);
				/**
				 * Sets the data about the preceding vehicle in the platoon. This data is usually
				 * received by means of wireless communications
				 */
				void setFrontVehicleData(double controllerAcceleration, double acceleration, double speed, double positionX, double positionY, double time);
				void setPrecedingVehicleData(double speed, double acceleration, double positionX, double positionY, double time);
				/**
				 * Gets the data about a vehicle. This can be used by a platoon leader in order to query for the acceleration
				 * before sending the data to the followers
				 */
				void getVehicleData(double &speed, double &acceleration, double &controllerAcceleration, double &positionX, double &positionY, double &time);

				/**
				 * Set the cruise control desired speed
				 */
				void setCruiseControlDesiredSpeed(double desiredSpeed);
				/**
				 * Set the currently active controller, which can be either the driver, the ACC or
				 * the CACC. CC is not mentioned because CC and ACC work together
				 *
				 * @param vehicleId the id of vehicle for which the active controller must be set
				 * @param activeController the controller to be activated: 0 for driver, 1 for
				 * ACC and 2 for CACC
				 */
				void setActiveController(int activeController);

				/**
				 * Returns the currently active controller
				 */
				int getActiveController();
				/**
				 * Set CACC constant spacing
				 *
				 * @param vehicleId the id of vehicle for which the constant spacing must be set
				 * @param spacing the constant spacing in meter
				 */
				void setCACCConstantSpacing(double spacing);

				/**
				 * Returns the CACC constant spacing
				 */
				double getCACCConstantSpacing();

				/**
				 * Sets all PATH's CACC and FAKED CACC parameters. Parameters set to negative values
				 * will remain untouched
				 */
				void setPathCACCParameters(double omegaN=-1, double xi=-1, double c1=-1, double distance=-1);

				/**
				 * Sets all Ploeg's CACCparameters. Parameters set to negative values
				 * will remain untouched
				 */
				void setPloegCACCParameters(double kp=-1, double kd=-1, double h=-1);

				/**
				 * Sets the headway time for the ACC
				 *
				 * @param vehicleId the id of the vehicle
				 * @param headway the headway time in seconds
				 */
				void setACCHeadwayTime(double headway);

				/**
				 * Enables/disables a fixed acceleration
				 *
				 * @param vehicleId the id of the vehicle
				 * @param activate activate (1) or deactivate (0) the usage of a fixed acceleration
				 * @param acceleration the fixed acceleration to be used if activate == 1
				 */
				void setFixedAcceleration(int activate, double acceleration);

				/**
				 * Returns whether a vehicle has crashed or not
				 *
				 * @param vehicleId the id of the vehicle
				 * @return true if the vehicle has crashed, false otherwise
				 */
				bool isCrashed();

				/**
				 * Set a fixed lane a car should move to
				 *
				 * @param laneIndex lane to move to, where 0 indicates the rightmost.
				 * @param safe whether changing lane should respect safety distance
				 * or simply avoid collisions
				 * Set the lane index to -1 to give control back to the human driver
				 */
				void setFixedLane(int8_t laneIndex, bool safe=false);

				/**
				 * Gets the data measured by the radar, i.e., distance and relative speed.
				 * This is basically what SUMO measures, so it gives back potentially
				 * infinite distance measurements. Taking into account that the maximum
				 * distance measurable of the Bosch LRR3 radar is 250m, when this
				 * method returns a distance value greater than 250m, it shall be
				 * interpreted like "there is nobody in front"
				 */
				void getRadarMeasurements(double &distance, double &relativeSpeed);

				void setLeaderVehicleFakeData(double controllerAcceleration, double acceleration, double speed);
				void setLeaderFakeData(double leaderSpeed, double leaderAcceleration);

				void setFrontVehicleFakeData(double controllerAcceleration, double acceleration, double speed, double distance);
				void setFrontFakeData(double frontDistance, double frontSpeed, double frontAcceleration);

				/**
				 * Gets the distance that a vehicle has to travel to reach the end of
				 * its route. Might be really useful for deciding when a car has to
				 * leave a platoon
				 */
				double getDistanceToRouteEnd();

				/**
				 * Gets the distance that a vehicle has traveled since the begin
				 */
				double getDistanceFromRouteBegin();

				/**
				 * Gets acceleration that the ACC has computed while the vehicle
				 * is controlled by the faked CACC
				 */
				double getACCAcceleration();
				/**
				 * Returns the vehicle type of a vehicle
				 */
				std::string getVType();
				/**
				 * Sets data information about a vehicle in the same platoon
				 */
				void setVehicleData(const struct Plexe::VEHICLE_DATA *data);
				/**
				 * Gets data information about a vehicle in the same platoon, as stored by this car
				 */
				void getVehicleData(struct Plexe::VEHICLE_DATA *data, int index);
				void getStoredVehicleData(struct Plexe::VEHICLE_DATA *data, int index);

				/**
				 * Determines whether PATH's and PLOEG's CACCs should use the controller
				 * or the real acceleration when computing the control action
				 * @param use if set to true, the vehicle will use the controller acceleration
				 */
				void useControllerAcceleration(bool use);

				/**
				 * If the vehicle is using the realistic engine model, this method
				 * returns the current gear and the engine RPM
				 * @param gear the current gear. if the realistic engine model is
				 * not used, this field is set to -1
				 * @param rpm the current engine rpm
				 */
				void getEngineData(int &gear, double &rpm);

				/**
				 * Activates or deactivates autofeeding, meaning that the user is not
				 * simulating inter-vehicle communication, so the CACCs will
				 * automatically take the required data from other vehicles automatically
				 * @param enable: boolean to enable or disable auto feeding
				 * @param leaderId: id of the leader vehicle. When disabling auto
				 * feeding, this parameter can be an empty string
				 * @param frontId: id of the front vehicle. When disabling auto
				 * feeding, this parameter can be an empty string
				 */
				void enableAutoFeed(bool enable, std::string leaderId="", std::string frontId="");

				/**
				 * Activates or deactivates prediction, i.e., interpolation of missing
				 * data for the control system
				 * @param enable: enable or disable prediction
				 */
				void usePrediction(bool enable);

				/**
				 * Adds a platoon member to this vehicle, usually considered to be the
				 * leader. Members are used to perform coordinated, whole-platoon lane
				 * changes
				 * @param memberId: sumo id of the member being added
				 * @param position: position (0-based) of the vehicle
				 */
				void addPlatoonMember(std::string memberId, int position);

				/**
				 * Removes a platoon member from this vehicle, usually considered to be the
				 * leader. Members are used to perform coordinated, whole-platoon lane
				 * changes
				 * @param memberId: sumo id of the member being removed
				 */
				void removePlatoonMember(std::string memberId);

				/**
				 * Enables/disables automatic, coordinated, whole-platoon lane changes.
				 * This function should be invoked on the leader which decides whether
				 * the platoon can gain speed by changing lane. The leader will then
				 * check whether lane changing is possible and, in case, do so
				 * @param enable: enable or disable automatic platoon lane changes
				 */
				void enableAutoLaneChanging(bool enable);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string nodeId;

				/**
				 * Tells to the CC mobility model the desired lane change action to be performed
				 *
				 * @param vehicleId the vehicle id to communicate the action to
				 * @param action the action to be performed. this can be either:
				 * 0 = driver choice: the application protocol wants to let the driver chose the lane
				 * 1 = management lane: the application protocol wants the driver to move the car
				 * to the management lane, i.e., the leftmost minus one
				 * 2 = platooning lane: the application protocol wants the driver to move the car
				 * to the platooning lane, i.e., the leftmost
				 * 3 = stay there: the application protocol wants the driver to keep the car
				 * into the platooning lane because the car is a part of a platoon
				 */
				void setLaneChangeAction(int action);
		};
		Vehicle vehicle(std::string nodeId) {
			return Vehicle(this, nodeId);
		}

		// Road methods
		std::list<std::string> getRoadIds();
		class Road {
			public:
				Road(TraCICommandInterface* traci, std::string roadId) : traci(traci), roadId(roadId) {
					connection = &traci->connection;
				}

				double getCurrentTravelTime();
				double getMeanSpeed();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string roadId;
		};
		Road road(std::string roadId) {
			return Road(this, roadId);
		}

		// Lane methods
		std::list<std::string> getLaneIds();
		class Lane {
			public:
				Lane(TraCICommandInterface* traci, std::string laneId) : traci(traci), laneId(laneId) {
					connection = &traci->connection;
				}

				std::list<Coord> getShape();
				std::string getRoadId();
				double getLength();
				double getMaxSpeed();
				double getMeanSpeed();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string laneId;
		};
		Lane lane(std::string laneId) {
			return Lane(this, laneId);
		}

		// Trafficlight methods
		class Trafficlight {
			public:
				Trafficlight(TraCICommandInterface* traci, std::string trafficLightId) : traci(traci), trafficLightId(trafficLightId) {
					connection = &traci->connection;
				}

				void setProgram(std::string program);
				void setPhaseIndex(int32_t index);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string trafficLightId;
		};
		Trafficlight trafficlight(std::string trafficLightId) {
			return Trafficlight(this, trafficLightId);
		}

		// Polygon methods
		std::list<std::string> getPolygonIds();
		void addPolygon(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<Coord>& points);
		class Polygon {
			public:
				Polygon(TraCICommandInterface* traci, std::string polyId) : traci(traci), polyId(polyId) {
					connection = &traci->connection;
				}

				std::string getTypeId();
				std::list<Coord> getShape();
				void setShape(const std::list<Coord>& points);
				void remove(int32_t layer);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string polyId;
		};
		Polygon polygon(std::string polyId) {
			return Polygon(this, polyId);
		}

		// Poi methods
		void addPoi(std::string poiId, std::string poiType, const TraCIColor& color, int32_t layer, const Coord& pos);
		class Poi {
			public:
				Poi(TraCICommandInterface* traci, std::string poiId) : traci(traci), poiId(poiId) {
					connection = &traci->connection;
				}

				void remove(int32_t layer);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string poiId;
		};
		Poi poi(std::string poiId) {
			return Poi(this, poiId);
		}

		// Junction methods
		std::list<std::string> getJunctionIds();
		class Junction {
			public:
				Junction(TraCICommandInterface* traci, std::string junctionId) : traci(traci), junctionId(junctionId) {
					connection = &traci->connection;
				}

				Coord getPosition();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string junctionId;
		};
		Junction junction(std::string junctionId) {
			return Junction(this, junctionId);
		}

		// Route methods
		std::list<std::string> getRouteIds();
		class Route {
			public:
				Route(TraCICommandInterface* traci, std::string routeId) : traci(traci), routeId(routeId) {
					connection = &traci->connection;
				}

				std::list<std::string> getRoadIds();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string routeId;
		};
		Route route(std::string routeId) {
			return Route(this, routeId);
		}

		// Vehicletype methods
		std::list<std::string> getVehicleTypeIds();

		// GuiView methods
		class GuiView {
			public:
				GuiView(TraCICommandInterface* traci, std::string viewId) : traci(traci), viewId(viewId) {
					connection = &traci->connection;
				}

				void setScheme(std::string name);
				void setZoom(double zoom);
				void setBoundary(Coord p1, Coord p2);
				void takeScreenshot(std::string filename = "");
				void trackVehicle(std::string vehicleId);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string viewId;
		};
		GuiView guiView(std::string viewId) {
			return GuiView(this, viewId);
		}


	private:
		TraCIConnection& connection;

		std::string genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		Coord genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		double genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		int32_t genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<std::string> genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<Coord> genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);

		typedef struct {
			int lane;
			bool safe;
			bool wait;
		} PlexeLaneChange;
		typedef std::map<std::string, PlexeLaneChange> PlexeLaneChanges;
		PlexeLaneChanges laneChanges;
		void __changeLane(std::string veh, int current, int direction, bool safe=true);
};

}

#endif