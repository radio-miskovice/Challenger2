#if !defined(COMMANDER_H)

class Commander {
  struct CommanderStatus {
    int active: 1 ;
  } status ;

  void activate();
  void deactivate();
  bool isActive() ;
  // void activateCommand( char );
  
};

extern Commander commander ;
#endif