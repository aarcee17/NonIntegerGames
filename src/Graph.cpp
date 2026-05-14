//============================================================================
// Name        : graph.cpp
// Author      : Suguman Bansal
// Version     :
// Copyright   : Your copyright notice
// Description : Class for Graph
//============================================================================

#include "common.h"
#include "Graph.h"

using namespace std;
using namespace boost;
using namespace boost::algorithm;

Graph::Graph(){
  this->numState = 0;
  this->initState = 0;
  this->maxWt = 0;
  this->maxPosWt = 0;
  this->maxNegWt = 0;
  this->stateToPlayer = {};
  this->transFunc = {};
  this->reach_objective = {};
  this->reach = false;
}


Graph::Graph(int num, int initial, int wt, unordered_map<int, int>* stateToP, unordered_map<int, vector< Transition*>>* transMap, unordered_map<int, bool>* reach_states, bool rflag){
  this->numState = num;
  this->initState = initial;
  this->maxWt = wt;
  this->maxPosWt = wt;
  this->maxNegWt = -wt;
  this->stateToPlayer = *stateToP;
  this->transFunc = *transMap;
  this-> reach_objective = *reach_states;
  this->reach = rflag;
}


Graph::Graph(string filename){

  int stateNum = 0;
  int initial = 0;
  int maxweight = 0;
  int maxposweight = 0;
  int maxnegweight = 0;
  unordered_map<int, int> stateToP = {};
  unordered_map<int, vector<Transition*>> transMap = {};
  unordered_map<int, bool> reach_states;;
  bool rflag = false;
  
  ifstream inFile;
  inFile.open(filename);
  if (!inFile){
    cerr << "Unable to open file" << endl;
    exit(1);
  }

  string output;

  int len;
  int src;
  int dest;
  int wt;
  int partition;
  
  while(!inFile.eof()){

    bool reachability = false;
    //output
    getline(inFile, output);
 
    trim(output);
 
    if (output[0] == '#'){
      //IGNORE COMMENT
    }
    else{
      //Split the sequence at the whitespace
      // If split-sequence has
      // (a). single element -- iniital state
      // (b). two elements -- state and player affiliation
      // (c). three elements
      //    (i). If last is R, then (state and player) and state is a reachability objective
      //    (ii) If last is not R, transition

      vector<string> splitparts;
      split(splitparts, output, [](char c){return c == ' ';});
      len = splitparts.size();

      switch(len){
      case 1:
	if (!(splitparts[0]=="" or splitparts[0]=="\n" or splitparts[0]=="\t" or splitparts[0]=="\r")){
	    initial = stoi(splitparts[0]);
	}	
	break;
	
      case 2:
	src = stoi(splitparts[0]);
	partition = stoi(splitparts[1]);
	//Set up the state-partition 
	stateToP.insert({src, partition});
	//Set up the transition relation
        transMap.insert({src, {}});
        stateNum +=1;
	reach_states[src] = false;
	break;
	
      case 3:
	if (splitparts[2]=="R"){
	  // Register the state
	  src = stoi(splitparts[0]);
	  partition = stoi(splitparts[1]);
	  //Set up the state-partition 
	  stateToP.insert({src, partition});
	  //Set up the transition relation
	  transMap.insert({src, {}});
	  stateNum +=1;
	  //Register src as a reachability objective
	  //reach_states.push_back(src);
	  reach_states[src] = true;
	  rflag = true;
	}
	else{	
	  src = stoi(splitparts[0]);	
	  dest = stoi(splitparts[1]);
	  wt = stoi(splitparts[2]);
	  //cout << src << " " << dest << " " << wt << endl;
	  Transition* tempTrans = new Transition(src, dest, wt);
	  //tempTrans->toString();
	  transMap[src].push_back(tempTrans);
	  if (abs(wt) > maxweight) maxweight = abs(wt);
	  if (wt > maxposweight) maxposweight = wt;
	  if (wt < maxnegweight) maxnegweight = wt;
	}
	break;	
      }
    }
  }

  inFile.close();
  
  this->numState = stateNum;
  this->initState = initial;
  this->maxWt = maxweight;
  this->maxPosWt = maxposweight;
  this->maxNegWt = maxnegweight;
  this->stateToPlayer = stateToP;
  this->transFunc = transMap;
  this->reach_objective = reach_states;
  this->reach = rflag;
}

Graph::~Graph(){
  //TODO
}
int Graph::getInitial(){
  return this->initState;
}

int Graph::getStateNum(){
  return this->numState;
}

int Graph::getWt(){
  return this->maxWt;
}

int Graph::getMaxPosWt(){
  return this->maxPosWt;
}

int Graph::getMaxNegWt(){
  return this->maxNegWt;
}

unordered_map<int, int>* Graph::getStateToPlayer(){
  return &(this->stateToPlayer);
}

unordered_map<int, vector<Transition*>>* Graph::getTrans(){
  return &(this->transFunc);
}

int Graph::getTransNum(){
  //TODO
}

unordered_map<int, bool>* Graph::getReachability(){
  return &(this->reach_objective);
}

bool Graph::isReach(){
  return this->reach;
}

void Graph::printInitial(){
  cout << "Initial state: " << this->getInitial() << endl;
}

void Graph::printMaxWt(){
  cout << "Max Wt: " << this->getWt() << endl;
}

void Graph::printStoPlayer(){
  cout << "State-Player mapping is " << endl;

  unordered_map<int, int>:: iterator p;
  unordered_map<int, int>* temp = this->getStateToPlayer();
  
  for (p = temp->begin(); p != temp->end(); p++){
    cout << p->first << ", " << p->second << endl;
  }
}

void Graph::printTrans(){
  cout << "Transition relation is " << endl;

  unordered_map<int, vector<Transition*>> :: iterator p;
  unordered_map<int, vector<Transition*>>* temp = this->getTrans();
  for (p = temp->begin(); p != temp->end(); p++){
    vector<Transition*> temptranslist= p->second;
    for (auto & element : temptranslist) {
      element->toString();
    }
  }
}

void Graph::printReachability(){

  cout << "Reachability relation is " << endl;

  unordered_map<int, bool> :: iterator p;
  unordered_map<int, bool>* temp = this->getReachability();
  //unordered_map<int, int>* temp = this->getReachability();
  for (p = temp->begin(); p != temp->end(); p++){
    cout << p->first << ", " << p->second << endl;
  }

  /*
  int numreach = temp->size();
  
  if (numreach == 0){
      return;
    }
  cout << "Reachability objectives:";
  for(int i=0; i< numreach; i++){
    cout << " " << temp->at(i);
  }
  cout << endl;
  */
  
}
void Graph::printAll(){
  this->printInitial();
  this->printStoPlayer();
  this->printTrans();
  this->printMaxWt();
  this->printReachability();
}
