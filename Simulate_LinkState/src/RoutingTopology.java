public class RoutingTopology {

    /*
     Topologin exempel med fem routrar och "cost"

     R1 -------3------- R3
      |  -              -|
      |    2-       -7   |
      1        R2        6
      |    3-       -5   |
      | -              - |
     R4 -------4--------R5
     */

    //0 = egen router
    //-1 = ingen väg, dvs router har inte max antal grannar
    private static Integer[][] topology = {
            {0,2,3,1,-1},
            {2,0,7,3,5},
            {3,7,0,-1,6},
            {1,3,-1,0,4},
            {-1,5,6,4,0}
    };

    public Integer NmbrOfRouters(){
        return topology[0].length;
    }

    //För att få cost för en routers länkar
    public Integer[] GetLinksCostAt(int index){
        Integer len = NmbrOfRouters();
        Integer[] routerNeighbours = new Integer[len];

        for(int row = 0; row< len; row++){
            if(index==row){
                for(int col = 0; col<len; col++){
                    routerNeighbours[col] = routerNeighbours[topology[index][col]];
                }
            }
        }
        return routerNeighbours;
    }
}
