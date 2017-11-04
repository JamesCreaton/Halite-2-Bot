#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"

std::vector<hlt::Move> moves;

void Dock(const hlt::Map& map, const hlt::Ship& ship, const hlt::Planet& planet)
{
	hlt::possibly<hlt::Move> move;
	//std::vector<const hlt::Entity*> objectsInWay = hlt::navigation::objects_between(map, ship.location, planet.location);

	//if (objectsInWay.size() > 0)
	//{
	//	//There are objects inbetween our path
	//	for (int i = 0; i < objectsInWay.size(); i++) {
	//		if (objectsInWay[i]->owner_id != ship.owner_id) {
	//			//Move to that object and attack
	//			move = hlt::navigation::navigate_ship_to_dock(map, ship, *objectsInWay[i], hlt::constants::MAX_SPEED);
	//		}
	//	}
	//}

	if (ship.can_dock(planet)) {
		moves.push_back(hlt::Move::dock(ship.entity_id, planet.entity_id));
		return;
	}
	move = hlt::navigation::navigate_ship_to_dock(map, ship, planet, hlt::constants::MAX_SPEED);

	if (move.second) {
		moves.push_back(move.first);
	}
}

void Attack(const hlt::Map& map, const hlt::Ship& ship, hlt::Ship& enemy)
{
	hlt::possibly<hlt::Move> move;
	move = hlt::navigation::navigate_ship_to_dock(map, ship, enemy, hlt::constants::MAX_SPEED);
	if (move.second) {
		moves.push_back(move.first);
	}
}

int main() {
	const hlt::Metadata metadata = hlt::initialize("JamesCreaton");
	const hlt::PlayerId player_id = metadata.player_id;

	const hlt::Map& initial_map = metadata.initial_map;

	// We now have 1 full minute to analyse the initial map.
	std::ostringstream initial_map_intelligence;
	initial_map_intelligence
		<< "width: " << initial_map.map_width
		<< "; height: " << initial_map.map_height
		<< "; players: " << initial_map.ship_map.size()
		<< "; my ships: " << initial_map.ship_map.at(player_id).size()
		<< "; planets: " << initial_map.planets.size();
	hlt::Log::log(initial_map_intelligence.str());

	for (;;) {
		moves.clear();
		const hlt::Map map = hlt::in::get_map();

		for (const hlt::Ship& ship : map.ships.at(player_id)) {
			if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
				continue;
			}

			//DO A CHECK FOR ENTITIES AROUND THE PLANETS I OWN AND SEE IF THERE ARE ENEMIES
			//IF THERE IS, ATTACK THEM

			std::vector<const hlt::Planet*> planetsByDistance = map.getPlanetsByDistance(ship.location);

			for (const hlt::Planet* planet : planetsByDistance) {
				if (planet->owned) {

					if (planet->owner_id == player_id) {
						if (!planet->is_full()) {
							Dock(map, ship, *planet);
							break;
						}
						continue;
					}

					std::vector<hlt::Ship> nearbyShips = map.NearbyEnemyShips(*planet, 6.0);

					//If there is only a small number of ships around a planet, move to the first one and attack it
					if (nearbyShips.size() > 3) {
						Attack(map, ship, nearbyShips[0]);
					}
				}

				Dock(map, ship, *planet);

				break;
			}
		}
		if (!hlt::out::send_moves(moves)) {
			hlt::Log::log("send_moves failed; exiting");
			break;
		}
	}
}