/*
 * valueiter.cpp
 *
 *  Created on: Oct 11, 2011
 *      Author: baj
 */

#include "dynamicprogramming.h"
#include "taxi.h"
#include "state.h"

#include <cassert>
#include <vector>

DPAgent::DPAgent(const bool test): OLAgent(test)
{
	ValueIteration();
}

DPAgent::~DPAgent()
{
	qtable_.save("dp");
}

Action DPAgent::plan(const State & state)
{
	return Greedy(state).second;
}

void DPAgent::ValueIteration()
{
	if (qtable_.load("dp")) return;

	int episodes = 0;

	do {
		double max_error = 0.0;

		for (int x = 0; x < TaxiEnv::SIZE; ++x) {
			for (int y = 0; y < TaxiEnv::SIZE; ++y) {
				for (int passenger = 0; passenger < 5; ++passenger) {
					for (int destination = 0; destination < 4; ++destination) {
						if (destination == passenger) continue;

						double error = UpdateValue(State(x, y, passenger, destination));

						if (error > max_error) {
							max_error = error;
						}
					}
				}
			}
		}

		cout << episodes++ << " " << max_error << endl;

		if (max_error < 0.01) {
			break;
		}
	} while (1);
}

double DPAgent::UpdateValue(const State & state)
{
	double error = 0.0;

	for (int i = 0; i < ActionSize; ++i) {
		error += UpdateQValue(state, Action(i));
	}

	return error;
}

pair<double, Action> DPAgent::Greedy(const State & state)
{
	double max = -1.0e6;
	Action best = Nil;

	for (int i = 0; i < ActionSize; ++i) {
		double tmp = qtable_[state][i];

		if (tmp > max) {
			max = tmp;
			best = Action(i);
		}
	}

	return make_pair(max, best);
}

double DPAgent::UpdateQValue(const State & state, Action action)
{
	static const double gamma = 1.0 - 1.0e-3;

	double tmp = TaxiEnv::Reward(state, action);

	vector<pair<State, double> > samples = TaxiEnv::Transition(state, action);

	for (int i = 0; i < int(samples.size()); ++i) {
		tmp += gamma * samples[i].second * Greedy(samples[i].first).first;
	}

	double error = abs(qtable_[state][action] - tmp);

	qtable_[state][action] = tmp;

	return error;
}
