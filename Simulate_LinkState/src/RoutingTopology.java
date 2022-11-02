//Petter Rignell - LinkSimulering
//2022-10-25

import java.util.Arrays;

public class RoutingTopology {

    /*
     Topologin exempel med fem routrar och "cost"

     R1 -------3------- R3
      |  -              -|
      |    2-       -7   |
      1        R2        |
      |    3-       -5   |
      | -              - |
     R4 -------4--------R5
     */

    //5x5 topologi matrix. Kan ändras med att lägga till/ta bort noder (routers)!
    //0 = egen router
    //-1 = ingen väg, dvs router har inte max antal grannar
    private int[][] topology = {
        {0,2,3,1,-1},
        {2,0,7,3,5},
        {3,7,0,-1,6},
        {1,3,-1,0,4},
        {-1,5,6,4,0},
    };

    public int getMatrixSize(){
        return topology.length;
    }

    //För att få cost för en routers länkar
    public int[] GetLinksCostAt(int index){
        int len = topology.length;
        int[] routerNeighbours = new int[len];
        try{
            for(int row = 0; row< len; row++){
                if(index==row){
                    for(int col = 0; col<len; col++){
                        routerNeighbours[col] = topology[index][col];
                    }
                }
            }
        }catch(Exception e){
            System.out.println(e + " links cost could not be added!");
        }

        return routerNeighbours;
    }

    public void UpdateTopology(int routerNmbr){
        for(int i = 0; i<topology.length; i++){
            if(i == routerNmbr-1){
                for(int j = 0; j< topology[0].length; j++){
                    topology[i][j] = -1;
                    topology[j][routerNmbr-1] = -1;
                }
            }
        }
    }

    public int[][] getTopology(){
        return topology;
    }

    public void setTopology(int[][] topology){
        this.topology=topology;
    }
}
