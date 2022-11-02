//Petter Rignell - LinkSimulering
//2022-10-25

public class Router{

    private String id;
    private int[] costsOfSending;
    private Boolean isFire;

    public Router(){ }

    public Router(String id, int[] costsOfSending, Boolean isFire){
        this.id = id;
        this.costsOfSending = costsOfSending;
        this.isFire = isFire;
    }

    public String getID(){
        return id;
    }

    public void setID(String routerID){
        this.id = routerID;
    }

    public int[] getCostsOfSending(){
        return costsOfSending;
    }

    public void setCostsOfSending(){
        this.costsOfSending = costsOfSending;
    }

    public Boolean getIsFire(){
        return isFire;
    }

    public void setIsFire(Boolean isFire){
        this.isFire = isFire;
    }
}
