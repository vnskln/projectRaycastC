struct Player {
    //player position and size on map
    float x;
    float y;
    float size;
    //-1 is left, +1 is right, 0 is not turning
    int turnDir;
    //-1 is backwards, +1 is forward, 0 is not moving
    int walkDIr;
    //angle between player front and 0
    float rotationAng;
    //angle change when turning
    float turnSpeedMod;
    //player speed
    float walkSpeedMod;
};

struct Ray {
    float angle;
    float hitX;
    float hitY;
    float distance;
    int hitVertical;
    int rayUp;
    int rayDown;
    int rayLeft;
    int rayRight;
    int hitMaterial;
};
