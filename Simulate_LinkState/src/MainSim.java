//Petter Rignell, Reem Mohamed - LinkSimulering
//2022-10-25
import java.util.Scanner;

public class MainSim {
    
    public static void main(String[] args){
        RouterManager routerManager = new RouterManager();
        Scanner scanner = new Scanner(System.in);
        String strFireAt = "";

        //Välj utgång
        System.out.println("Enter the exiting router (R1, R2...): ");
        String strExitRouter = scanner.nextLine();

        System.out.println("How many nodes should be impacted by fire?");
        int nmbrOfFires = Integer.parseInt(scanner.nextLine());

        for(int i = 0; i<nmbrOfFires; i++){
            System.out.println("Choose the first location of a fire at router (R1, R2...): ");
            strFireAt = scanner.nextLine();

            if (strFireAt.compareToIgnoreCase(strExitRouter) == 0){
                System.out.println("Could not add fire at exit router");
            }
            else {
                routerManager.AddFireAtRouter(strFireAt);
                String results = routerManager.GetShortestPaths(strExitRouter);
                System.out.print("Results: "+ results);
            }
        }
    }
}
