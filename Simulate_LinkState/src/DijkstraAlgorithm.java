//Petter Rignell, Reem Mohamed - LinkSimulering
//2022-11-01
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class DijkstraAlgorithm {

    private int[] prevNode;
    //variabler för start och end routrar
    private int start;
    private int end;

    //Räknar ut kortaste vägen och skriva ut den
    public String calcDijkstra(int source, int destination, int[][] topologyMatrix){
        int[] minDistance;
        int matrixSize = topologyMatrix.length;
        minDistance = new int[matrixSize];
        prevNode = new int[matrixSize];
        start = source-1;
        end = destination-1;

        Set<Integer> unvisitedNodes = new HashSet<>();

        if(topologyMatrix[start][start] == -1)
            return "Fire!";
        //Init: alla routrar är inte besökta i början och avståndet är oändligt
        for(int i = 0; i<matrixSize; i++){
            unvisitedNodes.add(i);
            if(i!=start)
                minDistance[i] = Integer.MAX_VALUE;
        }
        prevNode[start] = start;

        //Sparar minsta avståndet och förra (previous) routern
        while (!unvisitedNodes.isEmpty()){
            int currentNode = getNextNode(minDistance, unvisitedNodes);
            if(currentNode == -1) //ingen väg
                break;

            unvisitedNodes.remove(currentNode);

            for(int i = 0; i< matrixSize; i++){
                if(unvisitedNodes.contains(i) && topologyMatrix[currentNode][i]>0){

                    int newDistance = minDistance[currentNode] + topologyMatrix[currentNode][i];

                    if(newDistance < minDistance[i]){
                        minDistance[i] = newDistance;
                        prevNode[i] = currentNode;
                    }
                }
            }
        }
        return getShortestPathString(matrixSize, topologyMatrix);
    }

    //Få fram nästa router på den givna listan på routrarna
    private int getNextNode(int[] minDistance, Set<Integer> nodeOrder){
        int min = Integer.MAX_VALUE;
        int newNode = -1;

        for(int i: nodeOrder){
            if(minDistance[i]<min){
                newNode = i;
                min = minDistance[i];
            }
        }
        return newNode;
    }

    //Returnerar en sträng på hela vägen från "start" till "end" routern
    private String getShortestPathString(int len, int[][] topMatrix){
        int source = start+1;
        int destination = end+1;
        int i = end;
        String reversedPath = destination + "R";
        String path = "";

        while(true){
            if(prevNode[i]==i)
                break;

            reversedPath += " >-- " + (prevNode[i] +1) + "R";
            i = prevNode[i];
        }

        for(int j = reversedPath.length()-1; j >= 0; j--){
            path += reversedPath.charAt(j) + "";
        }

        return path;
    }

}

