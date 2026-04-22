//============================================================================
// Name        : game.cpp
// Author      : Suguman Bansal
// Version     :
// Copyright   : Your copyright notice
// Description : Class for Games
//============================================================================

#include "Game.h"
#include "Graph.h"
#include "Transition.h"
#include "common.h"

using namespace std;

string stringify(int graph_state, int comp_state) {
  string sep = "_";
  // cout << to_string(graph_state) + sep + to_string(comp_state) << endl;
  return to_string(graph_state) + sep + to_string(comp_state);
}

vector<string> split(const string &s) {
  char delim = '_';
  vector<string> result;
  stringstream ss(s);
  string item;

  while (getline(ss, item, delim)) {
    result.push_back(item);
  }
  return result;
}

Game::Game() {
  this->initial = "NA";
  this->winning = {};
  this->stateToPlayer = {};
  this->transFunc = {};
  this->reverseFunc = {};
  this->allstates = 0;
  this->states_at_first_win = -1;
  this->states_at_et = -1;
  this->construction_et = false;
}

Game::Game(Graph *gg, int df, int precision, int threshold, string relation) {

  //******* Begin: Initialize********//

  this->initial = "NA";
  this->winning = {};
  this->stateToPlayer = {};
  this->transFunc = {};
  this->reverseFunc = {};
  this->allstates = 0;
  this->states_at_first_win = -1;
  this->states_at_et = -1;
  this->construction_et = false;

  unordered_map<int, vector<Transition *>> *transF = gg->getTrans();
  unordered_map<int, int> *statePlayerID = gg->getStateToPlayer();
  unordered_map<int, bool> *reachability = gg->getReachability();

  unordered_map<string, int> statetoplayeraux = {};
  unordered_map<string, vector<string>> transfuncaux = {};
  unordered_map<string, vector<string>> reversefuncaux = {};
  unordered_map<string, string> winningaux = {};
  int numstategame = 0;

  int resolution_inverse = int(pow(2, df + precision));
  int dffactor = int(pow(2, df));
  //******* End: Initialize********//

  //******* Begin: Get bounds for comparator automata********//

  int maxWt = gg->getWt();
  int maxPosWt = gg->getMaxPosWt();
  int maxNegWt = gg->getMaxNegWt();

  // Current assumptions:
  //      (a). Lower bound approximation (for geq)
  //           Will be different for upper bound approximation (for leq)
  //      (b). threshold = 0

  int res_temp = int(pow(2, 2 * df + precision));

  // Asymmetric bounds: use weight-specific [L,U] instead of symmetric max-weight bounds.
  int lowerbound = maxNegWt * res_temp;
  int upperbound = maxPosWt * res_temp + int(pow(2, df));

  cout << "Optimization: Tighter bounds enabled." << endl;
  cout << "  Old theoretical bounds: " << -1 * maxWt * res_temp << " to "
       << 1 * maxWt * res_temp + int(pow(2, df)) << endl;
  cout << "  New tighter bounds    : " << lowerbound << " to " << upperbound
       << endl;

  cout << "Comparator range  : " << lowerbound << " to " << upperbound << endl;

  //******* End: Get bounds for comparator automata*********//

  // ******* Begin : initial state of game ********//
  string init = stringify(gg->getInitial(), 0);
  this->initial = init;

  // cout << init << endl;

  // ******* End : initial state of game ********//

  // ******* Begin : make all states  ********//

  // Maintaining statestack to maintain all states that have to be explored
  // Mainitaining isstate to maintain all states that have already been explored
  queue<string> statestack;
  unordered_map<string, bool> isstate;

  // Special state : init
  statestack.push(init);
  numstategame += 1;
  isstate[init] = true;
  statetoplayeraux[init] = statePlayerID->at(gg->getInitial());
  winningaux[init] = "";
  reversefuncaux[init] = {};

  while (!statestack.empty()) {
    string state = statestack.front();
    statestack.pop();
    // cout << state << endl;
    // Parts of current state
    vector<string> statetemp = split(state);
    int cur_state = stoi(statetemp[0]);
    int cur_comparator = stoi(statetemp[1]);
    // Used to store all outgoing neighbours
    vector<string> deststatelist = {};

    vector<Transition *> translist = transF->at(cur_state);

    for (auto &trans : translist) {
      //*****Begin: Make new state*****//
      // (a). Find the next state in the graph
      // (b). Find the next state in the comparator

      // (a). Find the next state in the graph
      int next_src = trans->getDest();

      // (b). Calculate the next state in the comparator

      int wt = trans->getWt();

      int next_comparator = 0;
      int next_comparator_temp = cur_comparator + wt * resolution_inverse +
                                 int(cur_comparator / dffactor);
      if (next_comparator_temp > upperbound) {
        next_comparator = upperbound;
      } else if (next_comparator_temp < lowerbound) {
        next_comparator = lowerbound;
      } else {
        next_comparator = next_comparator_temp;
      }

      // Make the destination state using (a) and (b)
      string new_state = stringify(next_src, next_comparator);

      //*****End: Make new state*****//

      // Check if new_state is indeed a new state
      // It is a new state if new_state is not already a key in isstate
      // If new_state is a new state, then add it to statestack
      // If new_state is a winning state, add it to winning_state_stack
      try {
        bool boolval = isstate.at(new_state);
      } catch (const std::out_of_range) {
        isstate[new_state] = true;
        statetoplayeraux[new_state] = statePlayerID->at(next_src);
        reversefuncaux[new_state] = {};

        //****** Begin: make it a winning state **********//

        if (!gg->isReach()) {
          if (relation == "geq" and next_comparator == upperbound) {
            winningaux[new_state] = "W";
            if (this->states_at_first_win == -1) this->states_at_first_win = numstategame;
          } else if (relation == "leq" and next_comparator == lowerbound) {
            winningaux[new_state] = "W";
            if (this->states_at_first_win == -1) this->states_at_first_win = numstategame;
          } else {
            winningaux[new_state] = "";
          }
        } else {
          // if (next_src in reachability)
          // same conditions as above
          // else, state is not a winning state

          if (reachability->at(cur_state)) {
            // cout << cur_state << endl;
            // cout << "Yo" << endl;
            reachability->at(next_src) = true;
          }

          if (reachability->at(next_src)) {
            // new_src is a reachability objective
            if (relation == "geq" and next_comparator == upperbound) {
              winningaux[new_state] = "W";
              if (this->states_at_first_win == -1) this->states_at_first_win = numstategame;
            } else if (relation == "leq" and next_comparator == lowerbound) {
              winningaux[new_state] = "W";
              if (this->states_at_first_win == -1) this->states_at_first_win = numstategame;
            } else {
              winningaux[new_state] = "";
            }
          } else {
            winningaux[new_state] = "";
          }
        }
        //****** End: make it a winning state **********//

        statestack.push(new_state);
        numstategame += 1;
      }
      deststatelist.push_back(new_state);
      reversefuncaux[new_state].push_back(state);
    }
    transfuncaux[state] = deststatelist;
  }
  this->stateToPlayer = statetoplayeraux;
  this->transFunc = transfuncaux;

  // cout << "Completed while" << endl;

  /*
  //Make reverse transitions
  unordered_map<string, vector<string>> :: iterator p;

  for (p = transfuncaux.begin(); p != transfuncaux.end(); p++){
    string srcstate = p->first;
    vector<string> temptranslist= p->second;
    for (auto & element : temptranslist){
      try{
        reversefuncaux[element].push_back(srcstate);
        }
      catch(const std::out_of_range){
        reversefuncaux[element].push_back(srcstate);
      }
    }
  }
  */

  this->reverseFunc = reversefuncaux;
  this->winning = winningaux;
  this->allstates = numstategame;
}

Game::~Game() {
  // TODO
}

// Priority-guided constructor: min-heap on (U - comparator) with interleaved backward propagation.
Game::Game(Graph *gg, int df, int precision, int threshold,
           string relation, bool use_heuristic, int playerid) {

  this->initial = "NA";
  this->winning = {};
  this->stateToPlayer = {};
  this->transFunc = {};
  this->reverseFunc = {};
  this->allstates = 0;
  this->states_at_first_win = -1;
  this->states_at_et = -1;
  this->construction_et = false;

  if (!use_heuristic) return;

  unordered_map<int, vector<Transition *>> *transF = gg->getTrans();
  unordered_map<int, int> *statePlayerID = gg->getStateToPlayer();
  unordered_map<int, bool> *reachability = gg->getReachability();

  unordered_map<string, int> statetoplayeraux = {};
  unordered_map<string, vector<string>> transfuncaux = {};
  unordered_map<string, vector<string>> reversefuncaux = {};
  unordered_map<string, string> winningaux = {};
  int numstategame = 0;

  int resolution_inverse = int(pow(2, df + precision));
  int dffactor = int(pow(2, df));

  int maxPosWt = gg->getMaxPosWt();
  int maxNegWt = gg->getMaxNegWt();
  int maxWt = gg->getWt();
  int res_temp = int(pow(2, 2 * df + precision));

  int lowerbound = maxNegWt * res_temp;
  int upperbound = maxPosWt * res_temp + int(pow(2, df));

  cout << "Heuristic mode    : Priority-guided construction (A* style)" << endl;
  cout << "Comparator range  : " << lowerbound << " to " << upperbound << endl;

  string init = stringify(gg->getInitial(), 0);
  this->initial = init;

  // Min-heap on (upperbound - comparator): states closest to winning are expanded first.
  priority_queue<
    pair<int,string>,
    vector<pair<int,string>>,
    greater<pair<int,string>>
  > pq;

  unordered_map<string, bool> isstate;

  pq.push({upperbound - 0, init});
  numstategame += 1;
  isstate[init] = true;
  statetoplayeraux[init] = statePlayerID->at(gg->getInitial());
  winningaux[init] = "";
  reversefuncaux[init] = {};

  unordered_map<string, int> numtrans_back;    // remaining non-winning successors per state
  unordered_map<string, bool> fully_expanded;  // true once a state has been popped and processed
  queue<string> win_frontier;
  bool initial_wins = false;

  numtrans_back[init] = 0;
  fully_expanded[init] = false;

  while (!pq.empty() && !initial_wins) {
    auto [dist, state] = pq.top();
    pq.pop();

    vector<string> statetemp = split(state);
    int cur_state = stoi(statetemp[0]);
    int cur_comparator = stoi(statetemp[1]);
    vector<string> deststatelist = {};
    vector<Transition *> translist = transF->at(cur_state);
    int new_succs = 0;

    for (auto &trans : translist) {
      int next_src = trans->getDest();
      int wt = trans->getWt();

      int next_comparator_temp = cur_comparator + wt * resolution_inverse +
                                 int(cur_comparator / dffactor);
      int next_comparator = 0;
      if (next_comparator_temp > upperbound)      next_comparator = upperbound;
      else if (next_comparator_temp < lowerbound) next_comparator = lowerbound;
      else                                        next_comparator = next_comparator_temp;

      string new_state = stringify(next_src, next_comparator);

      try {
        bool boolval = isstate.at(new_state);
      } catch (const std::out_of_range) {
        isstate[new_state] = true;
        statetoplayeraux[new_state] = statePlayerID->at(next_src);
        reversefuncaux[new_state] = {};
        fully_expanded[new_state] = false;
        numtrans_back[new_state] = 0;

        // Mark winning
        bool is_win = false;
        if (!gg->isReach()) {
          if (relation == "geq" && next_comparator == upperbound) is_win = true;
          else if (relation == "leq" && next_comparator == lowerbound) is_win = true;
        } else {
          if (reachability->at(cur_state)) reachability->at(next_src) = true;
          if (reachability->at(next_src)) {
            if (relation == "geq" && next_comparator == upperbound) is_win = true;
            else if (relation == "leq" && next_comparator == lowerbound) is_win = true;
          }
        }

        if (is_win) {
          winningaux[new_state] = "W";
          if (this->states_at_first_win == -1)
            this->states_at_first_win = numstategame;
          win_frontier.push(new_state);
        } else {
          winningaux[new_state] = "";
        }

        pq.push({upperbound - next_comparator, new_state});
        numstategame += 1;
      }
      deststatelist.push_back(new_state);
      reversefuncaux[new_state].push_back(state);
      new_succs++;
    }

    transfuncaux[state] = deststatelist;
    fully_expanded[state] = true;

    numtrans_back[state] = new_succs;

    // Backward propagation: system wins on ANY winning successor; adversary on ALL.
    while (!win_frontier.empty()) {
      string wstate = win_frontier.front();
      win_frontier.pop();

      vector<string> rev_list;
      try { rev_list = reversefuncaux.at(wstate); }
      catch (const std::out_of_range) { continue; }

      for (auto &pred : rev_list) {
        if (winningaux.count(pred) && winningaux[pred] == "W") continue;

        int pred_player = statetoplayeraux.count(pred) ? statetoplayeraux[pred] : -1;
        if (pred_player == -1) continue;

        bool mark_winning = false;
        if (pred_player == playerid) {
          mark_winning = true;
        } else {
          if (fully_expanded.count(pred) && fully_expanded[pred]) {
            if (numtrans_back[pred] > 0) numtrans_back[pred]--;
            if (numtrans_back[pred] == 0) mark_winning = true;
          }
        }

        if (mark_winning) {
          winningaux[pred] = "W";
          win_frontier.push(pred);
          if (pred == init) {
            initial_wins = true;
            this->states_at_et = numstategame;
            this->construction_et = true;
          }
        }
      }
    }
  }

  this->stateToPlayer = statetoplayeraux;
  this->transFunc = transfuncaux;
  this->reverseFunc = reversefuncaux;
  this->winning = winningaux;
  this->allstates = numstategame;
}

string Game::getInitial() { return this->initial; }

int Game::getStatesAtFirstWin() { return this->states_at_first_win; }
int Game::getStatesAtET()       { return this->states_at_et; }
bool Game::getConstructionET()  { return this->construction_et; }

unordered_map<string, string> *Game::getWinning() { return &(this->winning); }

unordered_map<string, int> *Game::getStateToPlayer() {
  return &(this->stateToPlayer);
}

unordered_map<string, vector<string>> *Game::getTrans() {
  return &(this->transFunc);
}

unordered_map<string, vector<string>> *Game::getRevTrans() {
  return &(this->reverseFunc);
}

int Game::getallstates() { return this->allstates; }

void Game::printInitial() {
  cout << "Initial state is " << this->getInitial() << endl;
}

void Game::printWinning() {
  cout << "Winning states are:" << endl;
  unordered_map<string, string>::iterator p;
  unordered_map<string, string> *temp = this->getWinning();
  for (p = temp->begin(); p != temp->end(); p++) {
    cout << p->first << ", " << p->second << endl;
  }
}

void Game::printStoPlayer() {
  cout << "State-Player mapping is " << endl;

  unordered_map<string, int>::iterator p;
  unordered_map<string, int> *temp = this->getStateToPlayer();
  for (p = temp->begin(); p != temp->end(); p++) {
    cout << p->first << ", " << p->second << endl;
  }
}

void Game::printTrans() {
  cout << "Transition relation is " << endl;

  unordered_map<string, vector<string>>::iterator p;
  unordered_map<string, vector<string>> *temp = this->getTrans();

  string printbuffer = "";
  int counter = 1;

  for (p = temp->begin(); p != temp->end(); p++) {
    vector<string> temptranslist = p->second;
    for (auto &element : temptranslist) {
      printbuffer += p->first + "-->" + element + "\n";
      counter += 1;
      if (counter % 1000 == 0) {
        cout << printbuffer;
        printbuffer = "";
      }
    }
  }
  cout << printbuffer;
}

void Game::printRevTrans() {
  cout << "Transition reverse relation is " << endl;

  unordered_map<string, vector<string>>::iterator p;
  unordered_map<string, vector<string>> *temp = this->getRevTrans();

  for (p = temp->begin(); p != temp->end(); p++) {
    vector<string> temptranslist = p->second;
    for (auto &element : temptranslist) {
      cout << p->first << "-->" << element << endl;
    }
  }
}

void Game::printstatenum() { cout << this->allstates << endl; }

void Game::printAll() {
  printInitial();
  printWinning();
  printStoPlayer();
  printTrans();
}

void Game::modifywinning(string state, string gotostate) {
  (this->winning)[state] = gotostate;
}

bool Game::reachabilitygame(int player, bool early_termination) {

  unordered_map<string, vector<string>> *reverse_map = this->getRevTrans();
  unordered_map<string, vector<string>> *map = this->getTrans();
  unordered_map<string, int> *statetoplayer = this->getStateToPlayer();
  unordered_map<string, string> *winning = this->getWinning();
  string initial = this->getInitial();

  // Initially, we assume that the player doesn't win
  bool playerwins = false;

  // 1. Count the number of outgoing transitions from each state
  // 2. Insert the winning states into the stackstack
  queue<string> statestack;
  unordered_map<string, int> numtrans;
  unordered_map<string, vector<string>>::iterator p;
  for (p = map->begin(); p != map->end(); p++) {
    string statetempstring = p->first;
    numtrans[statetempstring] = (p->second).size();

    if (winning->at(statetempstring) == "W") {
      numtrans[statetempstring] = 0;
      statestack.push(statetempstring);
    }
  }

  while (!statestack.empty()) {
    string state = statestack.front();
    statestack.pop();
    vector<string> revtranslist;
    try {
      revtranslist = reverse_map->at(state);
    } catch (const std::out_of_range) {
      continue;
    }
    for (auto &element : revtranslist) {
      int statebelongsto = statetoplayer->at(element);
      if (statebelongsto == player) {
        // then element is a  winning state.
        // if element hasn't been visited before, then add to stack.
        // element hasn't been visited before, its numtrans != 0
        if (numtrans[element] != 0) {
          statestack.push(element);
          numtrans[element] = 0;
          this->modifywinning(element, state);
        }
        if (element == initial) {
          // player has won, as it is visiting  initial state, controlled by the
          // player
          playerwins = true;
          if (early_termination) {
            return playerwins;
          }
        }
      } else { // statebelongs to environment
        // element will be winning only when numtans turns 0
        // add element to stack only the first time numtrans turns 0
        if (numtrans[element] != 0) {
          numtrans[element] = numtrans[element] - 1;
          if (numtrans[element] ==
              0) { // numtrans becomes 0 for the first time  winning state
            statestack.push(element);
            if (element == initial) {
              playerwins = true;
              if (early_termination) {
                return playerwins;
              }
            }
          }
        }
      }
    } /*for ends*/
  }   /* while ends*/
  return playerwins;
}

void Game::rawprint(int player) {

  unordered_map<string, string> *wmap = this->getWinning();
  unordered_map<string, int> *ptosmap = this->getStateToPlayer();

  string state;
  string deststate;
  vector<string> scomp = {};
  vector<string> dcomp = {};
  int temp;
  string printbuffer = "";
  int counter = 1;

  unordered_map<string, string>::iterator p;
  for (p = wmap->begin(); p != wmap->end(); p++) {
    state = p->first;
    if (ptosmap->at(state) == player) {
      scomp = split(state);
      deststate = wmap->at(state);
      dcomp = split(deststate);
      temp = dcomp.size();
      if (temp == 2) {
        printbuffer += scomp[0] + ", " + scomp[1] + " --> " + dcomp[0] + ", " +
                       dcomp[1] + "\n";
        // cout << scomp[0] << ", " << scomp[1] << " --> " << dcomp[0] << ", "
        // << dcomp[1] << endl;
      }
      if (temp == 1) {

        printbuffer +=
            scomp[0] + ", " + scomp[1] + " --> " + "Any action " + "\n";
        // cout << scomp[0] << ", " << scomp[1] << " --> "  << "Any action" <<
        // endl;
      }
      if (temp == 0) {
      }
    }
    counter += 1;
    if (counter % 50000) {
      cout << printbuffer;
      printbuffer = "";
    }
  }
  cout << printbuffer;
}
