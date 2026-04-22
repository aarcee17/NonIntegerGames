/*
 * game.h
 *
 * Created on: October 31, 2019
 * Author: Suguman Bansal
 */

#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "Graph.h"
#include "Transition.h"

using namespace std;

class Game{

 public:
  Game();
  // Original constructor (BFS construction)
  Game(Graph* gg, int df, int precision, int threshold, string relation);
  // Heuristic constructor (priority-guided construction + interleaved solving)
  Game(Graph* gg, int df, int precision, int threshold, string relation, bool use_heuristic, int playerid);
  virtual ~Game();

  //playing game
  bool reachabilitygame(int player, bool early_termination);
  
  //Modify winning
  void modifywinning(string state, string gotostate);
  
  //Print winning strategie function(s)
  void rawprint(int player);
  
  //Access functions
  string getInitial();
  unordered_map<string, string>* getWinning();
  unordered_map<string, int>* getStateToPlayer();
  unordered_map<string, vector<string>>* getTrans();
  unordered_map<string, vector<string>>* getRevTrans();
  int getallstates();
  int getStatesAtFirstWin();      // states constructed when first winning state found
  int getStatesAtET();            // states constructed when early termination triggered
  bool getConstructionET();       // whether ET was triggered during construction
  
  //Print functions
  void printInitial();
  void printWinning();
  void printStoPlayer();
  void printTrans();
  void printRevTrans();
  void printstatenum();
  void printAll();
  
  
 private:
  string initial;
  unordered_map<string, string> winning;
  unordered_map<string, int> stateToPlayer;
  unordered_map<string, vector<string>> transFunc;
  unordered_map<string, vector<string>> reverseFunc;
  int allstates;
  // Instrumentation fields
  int states_at_first_win;   // how many states built when first winning state found
  int states_at_et;          // how many states built when ET triggered (heuristic mode)
  bool construction_et;      // true if construction stopped early due to ET
};


#endif 
