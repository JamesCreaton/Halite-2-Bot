#pragma once

#include "map.hpp"
#include "types.hpp"
#include "ship.hpp"
#include "planet.hpp"

namespace hlt {
	class Map {
	public:
		int map_width, map_height;

		std::unordered_map<PlayerId, std::vector<Ship>> ships;
		std::unordered_map<PlayerId, entity_map<unsigned int>> ship_map;

		std::vector<Planet> planets;
		entity_map<unsigned int> planet_map;

		Map(int width, int height);

		Ship & Map::get_shipNonConst(PlayerId player_id, EntityId ship_id) {
			return ships.at(player_id).at(ship_map.at(player_id).at(ship_id));
		}

		Planet& Map::get_planetNonConst(EntityId planet_id) {
			return planets.at(planet_map.at(planet_id));
		}

		const Ship& get_ship(const PlayerId player_id, const EntityId ship_id) const{
			return ships.at(player_id).at(ship_map.at(player_id).at(ship_id));
		}

		const Ship* get_ship(const PlayerId player_id, const EntityId ship_id, bool a_bool) const {
			return &ships.at(player_id).at(ship_map.at(player_id).at(ship_id));
		}

		const Planet& get_planet(const EntityId planet_id) const {
			return planets.at(planet_map.at(planet_id));
		}

		const int PlanetsOwned()const {
			int counter = 0;

			for (unsigned int i = 0; i < planets.size(); i++) {
				if (planets[i].owned) {
					counter++;
				}
			}

			const int numPlanets = counter;
			return numPlanets;
		}


		const std::vector<const hlt::Planet*> getPlanetsByDistance(const hlt::Location a_Position) const {
			std::vector<double> distances = std::vector<double>(planets.size());
			for (unsigned int i = 0; i < planets.size(); i++) {
				distances[i] = a_Position.get_distance_to(planets[i].location);
			}
			std::vector<const hlt::Planet*> planetsSorted = std::vector<const hlt::Planet*>(planets.size());
			for (unsigned int q = 0; q < planets.size(); q++) {
				int index = -1;
				double smallest = 9999;
				for (unsigned int i = 0; i < distances.size(); i++) {
					if (distances[i] < smallest) {
						smallest = distances[i];
						index = i;
					}
				}
				planetsSorted[q] = &planets[index];
				distances[index] = 9999;
			}
			return planetsSorted;
		}

		//const hlt::Ship& GetClosestEnemy(const hlt::Map& a_map, const hlt::Location a_location, const hlt::PlayerId a_enemyID)const {
		//	std::vector<double> distances = std::vector<double>(ships.at(a_enemyID).size());
		//	for (unsigned int i = 0; i < ships.at(a_enemyID).size(); i++) {
		//		distances[i] = a_location.get_distance_to(ships.at(a_enemyID)[i].location);
		//	}

		//	std::vector<const hlt::Ship*> shipsSorted = std::vector<const hlt::Ship*>(ships.at(a_enemyID).size());
		//	for (unsigned int q = 0; q < planets.size(); q++) {
		//		int index = -1;
		//		double smallest = 9999;
		//		for (unsigned int i = 0; i < distances.size(); i++) {
		//			if (distances[i] < smallest) {
		//				smallest = distances[i];
		//				index = i;
		//			}
		//		}
		//		shipsSorted[q] = &ships.at(a_enemyID)[index];
		//		distances[index] = 9999;
		//	}
		//	return &shipsSorted[0];
		//}

		const std::vector<const hlt::Ship*> GetDockingShips(const hlt::Planet& a_planet, double distanceToSearch, const hlt::Map& a_map) const
		{
			//Get all ships of the planets owner that are withing a certain distance of the current planet we're looking at
			//then check to see if any of them are currently in the docking stage
			//add them to a vector
			//return that vector

			std::vector<const hlt::Ship*> dockingShips = std::vector<const hlt::Ship*>();

			for (int i = 0; i < ships.at(a_planet.owner_id).size(); i++)
			{
				const hlt::Ship* currentShip = get_ship(a_planet.owner_id, i, true);

				if (currentShip->location.get_distance_to(a_planet.location) <= 2)
				{
					if (currentShip->docking_status == ShipDockingStatus::Docking)
					{
						dockingShips.push_back(currentShip);
					}
				}
			}
			return dockingShips;
		}

	};
}