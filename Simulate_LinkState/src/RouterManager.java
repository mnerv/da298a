//Petter Rignell - LinkSimulering
//2022-10-25

import java.io.Console;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class RouterManager {

    private List<Router> routers;
    private RoutingTopology rTop;

    public RouterManager(){
        ConfigureRouters();
    }

    //Skapa alla routers med id
    private Boolean ConfigureRouters(){
        routers = new ArrayList<Router>();
        rTop = new RoutingTopology();
        Router router;
        try{
            for(int i = 0; i< rTop.getMatrixSize(); i++){
                router = new Router("R"+i, rTop.GetLinksCostAt(i), false);
                routers.add(router);
            }
        }catch (Exception e){
            System.out.println(e + ". Could not configure router.");
        }
        return true;
    }

    //lägga till brand vid en nod/router
    //alla noder blir automatiskt medvetna om den nya topologin
    public Boolean AddFireAtRouter(String strRouter){
        int routerNmbr = Integer.parseInt(strRouter.substring(1));
        if (routerNmbr <= rTop.getMatrixSize()){
            try{
                routers.get(routerNmbr-1).setIsFire(true);
                rTop.UpdateTopology(routerNmbr);

            }catch(Exception e){
                System.out.println(rTop.getMatrixSize());
                System.out.println(e + ". Could not add fire to router");
            }
        }
        else {
            System.out.println("Could not add fire to not existing router");
        }

        return true;
    }

    //Kanske behövs
    public void CheckForFire(){
        for(int i = 0; i<routers.toArray().length; i++){
            if(routers.get(i).getIsFire()){
                rTop.UpdateTopology(i+1);
            }
        }
    }

    //Från router x till vald utgång
    public String GetShortestPaths(String strExitRouter){
        DijkstraAlgorithm dijA = new DijkstraAlgorithm();
        String strPath = "";
        int exitRouter = Integer.parseInt(strExitRouter.substring(1));

        for(int i = 0; i<routers.size(); i++){
            try {
                strPath += "\n Routingpath from R" + (i+1) + ": ";
                strPath += dijA.calcDijkstra(i+1, exitRouter, rTop.getTopology());
            }catch(Exception e){
                System.out.println("Mög i dijkstra. " + e);
            }
        }

        return strPath;
    }

    //Inte färdig - kanske behövs
    public boolean routerExists(String routerNmbr){
        boolean RouterExists = true;
        //...
        return RouterExists;
    }
}
