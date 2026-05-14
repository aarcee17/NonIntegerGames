/*
 * graph.h
 *
 * Created on: October 26, 2019
 * Author: Suguman Bansal
 */

#ifndef GRAPH_H
#define GRAPH_H

#include "common.h"
#include "Transition.h"

using namespace std;

class Graph{

 public:
  Graph();
  Graph(string filename);
  Graph(int num, int initial, int wt, unordered_map<int, int>* stateToP, unordered_map<int, vector< Transition*>>* transMap, unordered_map<int, bool>* reach_states, bool rfalg);
  virtual ~Graph();

  //Access functions
  int getInitial();
  int getStateNum();
  int getWt();
  int getMaxPosWt();
  int getMaxNegWt();
  unordered_map<int, int>* getStateToPlayer();
  unordered_map<int, vector< Transition*>>* getTrans();
  unordered_map<int, bool>* getReachability();
  bool isReach();
  
  //Functionality  
  int getTransNum();
  
  
  //Print functions
  void printInitial();
  void printStoPlayer();
  void printTrans();
  void printMaxWt();
  void printReachability();
  void printAll();
  
 private:
  int numState;
  int initState;
  int maxWt;
  int maxPosWt;
  int maxNegWt;
  unordered_map<int, int> stateToPlayer;
  unordered_map<int, vector<Transition*>> transFunc;
  unordered_map<int, bool> reach_objective;
  bool reach;
};


#endif 
