#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"
#include <chrono>


std::vector<hlt::Move> moves;

void Dock(const hlt::Map& map, const hlt::Ship& ship, const hlt::Planet& planet)
{
	hlt::possibly<hlt::Move> move;
	if (ship.can_dock(planet)) {
		moves.push_back(hlt::Move::dock(ship.entity_id, planet.entity_id));
		return;
	}
	move = hlt::navigation::navigate_ship_to_dock(map, ship, planet, hlt::constants::MAX_SPEED);

	if (move.second) {
		moves.push_back(move.first);
	}
}

void Attack(const hlt::Map& map, const hlt::Ship& ship, const hlt::Ship& enemy)
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

	auto t1 = std::chrono::high_resolution_clock::now();

	// We now have 1 full minute to analyse the initial map.
	std::ostringstream initial_map_intelligence;
	initial_map_intelligence
		<< "width: " << initial_map.map_width
		<< "; height: " << initial_map.map_height
		<< "; players: " << initial_map.ship_map.size()
		<< "; my ships: " << initial_map.ship_map.at(player_id).size()
		<< "; planets: " << initial_map.planets.size();
	hlt::Log::log(initial_map_intelligence.str());
	hlt::Log::log(std::string("test"));

	for (;;) {
		moves.clear();
		const hlt::Map map = hlt::in::get_map();

		for (const hlt::Ship& ship : map.ships.at(player_id)) {
			if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
				continue;
			}
			std::vector<const hlt::Planet*> planetsByDistance = map.getPlanetsByDistance(ship.location);

			for (const hlt::Planet* planet : planetsByDistance) {

				//Planet owned, so fill it
				if (planet->owned) {

					if (planet->owner_id == player_id) {
						if (!planet->is_full()) {
							Dock(map, ship, *planet);
							break;
						}
						continue;
					}
				}

				//Planet not owned, so attack ships on it 
				else
				{
					if (ship.can_dock(*planet)) {
						Dock(map, ship, *planet);
					}
					else {
						Attack(map, ship, map.get_ship(planet->owner_id, planet->docked_ships.at(0)));
					}
				}

				Dock(map, ship, *planet);
				break;
			}
		}
		auto t2 = std::chrono::high_resolution_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		hlt::Log::log("Turn Time: " + std::to_string(1000 * diff.count()));

		if (!hlt::out::send_moves(moves)) {
			hlt::Log::log("send_moves failed; exiting");
			break;
		}
	}
}