//Petter Rignell, Reem Mohamed - LinkSimulering
//2022-10-25
import java.util.Scanner;

public class MainSim {
    private RoutingTopology rTop = new RoutingTopology();

    public static void main(String[] args){
        RouterManager routerManager = new RouterManager();
        Scanner scanner = new Scanner(System.in);

        //Välj utgång
        System.out.println("Enter the exiting router (R1, R2...): ");
        String strExitRouter = scanner.nextLine();

        //Sätt eld
        System.out.println("Choose the location of the fire at router (R1, R2...): ");
        String strFireAt = scanner.nextLine();


        if (strFireAt.compareToIgnoreCase(strExitRouter) == 0){
            System.out.println("Could not add fire at exit router");
        }
        else {
            routerManager.AddFireAtRouter(strFireAt);
            //Vilka vägar är bäst för respektive router
            String results = routerManager.GetShortestPaths(strExitRouter);
            System.out.print("Results: "+ results);
        }

    }
}
