#if !defined(COMMANDER_H)

class Commander {
  struct CommanderStatus {
    int active: 1 ;
  } status ;

  bool isActive() ;
  void activateCommand( char );

};

extern Commander commander ;
#endif