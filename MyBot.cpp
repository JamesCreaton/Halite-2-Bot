#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"
#include <chrono>
#include <cassert>

std::vector<hlt::Move> moves;

void Dock(hlt::Map& map, hlt::Ship& ship, hlt::Planet& planet)
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

void Dock(hlt::Map map, hlt::Ship& ship, const hlt::EntityId entity_id)
{
	hlt::Planet planet = map.get_planet(entity_id);


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

void Attack(hlt::Map& map, hlt::Ship& ship, hlt::Ship* enemy)
{
	hlt::possibly<hlt::Move> move;
	move = hlt::navigation::navigate_ship_to_dock(map, ship, *enemy, hlt::constants::MAX_SPEED);
	if (move.second) {
		moves.push_back(move.first);
	}
}

void Attack(hlt::Map& map, hlt::Ship& ship, hlt::Ship& enemy)
{
	hlt::possibly<hlt::Move> move;
	move = hlt::navigation::navigate_ship_to_dock(map, ship, enemy, hlt::constants::MAX_SPEED);
	if (move.second) {
		moves.push_back(move.first);
	}
}

void SetEnemyID(const hlt::PlayerId& myID, hlt::PlayerId& enemyID)
{
	if (myID == 0) {
		enemyID = 1;
		return;
	}
	else if (myID == 1) {
		enemyID = 0;
		return;
	}
}

inline const char * const BoolToString(bool b)
{
	return b ? "true" : "false";
}

int main() {
	const hlt::Metadata metadata = hlt::initialize("James Creaton");
	const hlt::PlayerId player_id = metadata.player_id;
	const hlt::Map& initial_map = metadata.initial_map;

	bool notAlreadyGoingTo = true;
	bool rougeAttacker = false;
	bool onevsone = true;

	hlt::PlayerId enemyId = 10;

	// We now have 1 full minute to analyse the initial map.
	std::ostringstream initial_map_intelligence;
	initial_map_intelligence
		<< "width: " << initial_map.map_width
		<< "; height: " << initial_map.map_height
		<< "; players: " << initial_map.ship_map.size()
		<< "; my ships: " << initial_map.ship_map.at(player_id).size()
		<< "; planets: " << initial_map.planets.size();
	hlt::Log::log(initial_map_intelligence.str());
	hlt::Log::log("Rouge Attacker before:  " + std::to_string(rougeAttacker));

	if (initial_map.ship_map.size() > 2) {
		onevsone = false;
	}
	SetEnemyID(player_id, enemyId);

	auto t1 = std::chrono::high_resolution_clock::now();
	for (;;) {
		moves.clear();
		hlt::Map map = hlt::in::get_map();
		bool notAlreadyGoingTo = true;
		const hlt::Ship sneakyBeaky = map.ships.at(player_id).at(0);

		std::vector<hlt::EntityId> targets = std::vector<hlt::EntityId>();
		for (hlt::Ship& ship : map.ships.at(player_id)) {
			if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
				continue;
			}

			if (onevsone && sneakyBeaky.entity_id == ship.entity_id) {
				Attack(map, ship, map.get_shipNonConst(enemyId, map.ships.at(enemyId).front().entity_id));
				continue;
			}

			std::vector<const hlt::Planet*> planetsByDistance = map.getPlanetsByDistance(ship.location);

			for (const hlt::Planet* planet : planetsByDistance)
			{
				if (planet->owned)
				{
					if (planet->owner_id == player_id)
					{
						if (!planet->is_full())
						{
							Dock(map, ship, planet->entity_id);
							break;
						}
						continue;
					}
					else
					{
						//planet is owned but not by me
						Attack(map, ship, map.get_shipNonConst(planet->owner_id, planet->docked_ships[0]));
					}
				}
				else {
					for (int i = 0; i < targets.size(); i++)
					{
						if (targets.at(i) == planet->entity_id) {
							notAlreadyGoingTo = false;
							break;
						}
						notAlreadyGoingTo = true;
					}

					if (notAlreadyGoingTo) {
						Dock(map, ship, planet->entity_id);
						if (map.ship_map.at(player_id).size() == initial_map.ship_map.at(player_id).size()) {
							targets.push_back(planet->entity_id);
						}
					}
					else {
						continue;
					}
				}
				break;
			}
		}
		auto t2 = std::chrono::high_resolution_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		hlt::Log::log("Ships Time: " + std::to_string(1000 * diff.count()));
		hlt::Log::log("Rouge attacker: " + std::to_string(rougeAttacker));

		if (!hlt::out::send_moves(moves)) {
			hlt::Log::log("send_moves failed; exiting");
			break;
		}
	}
}