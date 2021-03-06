/*
 * pole.cpp
 *
 *  Created on: Sep 15, 2010
 *      Author: baj
 */

#include "taxi.h"

#include <cassert>

#include "state.h"

using namespace std;

const TaxiEnv::EnvModel *TaxiEnv::model;
int TaxiEnv::SIZE = 5;

double TaxiEnv::step(Action action)
{
	std::pair<State, double> sample = Sample(*state_, action);

	*state_ = sample.first;

	return sample.second;
}

std::pair<State, double> TaxiEnv::Sample(const State& state, Action action)
{
	double reward = Reward(state, action);
	State post_state(state);

	vector<pair<State, double> > samples = Transition(state, action);

	double cumulative_prob = 0.0, prob = drand48();

	for (uint i = 0; i < samples.size(); ++i) {
		cumulative_prob += samples[i].second;

		if (prob <= cumulative_prob) {
			post_state = samples[i].first;
			break;
		}
	}

	return make_pair(post_state, reward);
}

vector<pair<State, double> > TaxiEnv::Transition(const State & state, Action action)
{
	vector<pair<State, double> > samples;

	if (state.absorbing()) {
		samples.push_back(make_pair(state, 1.0));
	}
	else {
		switch (action) {
		case Putdown:
			if (!(state.passenger() == int(model->stops().size()) && Position(state.x(), state.y()) == model->stops()[state.destination()])) {
				samples.push_back(make_pair(state, 1.0));
			}
			else {
				State tmp(state);
				tmp.passenger() = tmp.destination();
				samples.push_back(make_pair(tmp, 1.0));
			}
			break;
		case Pickup:
			if (state.passenger() == int(model->stops().size()) || Position(state.x(), state.y()) != model->stops()[state.passenger()]) {
				samples.push_back(make_pair(state, 1.0));
			}
			else {
				State tmp(state);
				tmp.passenger() = int(model->stops().size());
				samples.push_back(make_pair(tmp, 1.0));
			}
			break;
		case South:
		case North:
		case West:
		case East:
		{
#if 0
			State tmp(state);

			TaxiEnv::Position location(state.x(), state.y());
			location = location + TaxiEnv::model->delta_[location.x][location.y][action];

			tmp.x() = location.x;
			tmp.y() = location.y;

			samples.push_back(make_pair(tmp, 1.0));
#else
			vector<Action> actions(3, action);
			vector<double> distribution(3, 0.1);
			vector<Position> locations(3, Position(state.x(), state.y()));

			distribution[0] = 0.8;

			switch (action) {
			case North: actions[1] = East; actions[2] = West; break;
			case South: actions[1] = East; actions[2] = West; break;
			case East: actions[1] = South; actions[2] = North; break;
			case West: actions[1] = South; actions[2] = North; break;
			default: assert(0); break;
			}

			State tmp(state);

			for (int i = 0; i < 3; ++i) {
			  Position delta = model->delta_[locations[i].x][locations[i].y][actions[i]];
				locations[i] = locations[i] + delta;

				tmp.x() = locations[i].x;
				tmp.y() = locations[i].y;

				samples.push_back(make_pair(tmp, distribution[i]));
			}
#endif
		}
			break;
		default: assert(0); break;
		}
	}

	return samples;
}

double TaxiEnv::Reward(const State & state, Action action)
{
	if (state.absorbing()) {
		return 0.0;
	}

	switch (action) {
	case Pickup:
		if (state.passenger() == int(model->stops().size()) || Position(state.x(), state.y()) != model->stops()[state.passenger()]) {
			return -10;
		}
		else {
			return -1;
		}
	case Putdown:
		if (state.passenger() == int(model->stops().size()) && Position(state.x(), state.y()) == model->stops()[state.destination()]) {
			return +20;
		}
		else {
			return -10;
		}
	case North:
	case South:
	case East:
	case West:
			return -1;
	default: assert(0); return -100000;
	}
}

void TaxiEnv::reset()
{
	Position taxi;
	int passenger, destination;

	do {
		taxi = get_random_position();
		passenger = get_random_stop();
		destination = get_random_stop();
	} while (passenger == destination);

	delete state_;
	state_ = new State(taxi.x, taxi.y, passenger, destination);
}
