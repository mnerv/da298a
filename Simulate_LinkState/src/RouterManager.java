import java.io.Console;
import java.util.List;

public class RouterManager {

    private List<Router> routers;

    //Skapa alla routers med id
    public Boolean ConfigureRouters(){
        RoutingTopology rTop= new RoutingTopology();
        Router router;

        try{
            for(int i = 0; i<rTop.NmbrOfRouters(); i++){
                router = new Router("R"+i, rTop.GetLinkCostAt(i), false);
                routers.add(router);
            }
        }catch (Exception e){
            System.out.println(e);
        }
        return true;
    }

    //lägga till brand vid en nod
    public Boolean AddFireToRouter(){
        //.....
        return true;
    }

    public Integer[][] GetShortestPath(){
        DijkstraAlgorithm dijA = new DijkstraAlgorithm();
        //dijA.CalcDijkstra();
        return null;
    }

}
