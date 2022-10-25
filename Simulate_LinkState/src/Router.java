public class Router {

    private String id;
    private Integer[] costsOfSending;
    private Boolean isFire;

    public Router(){ }

    public Router(String id, Integer[] costsOfSending, Boolean isFire){
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

    public Integer[] getCostsOfSending(){
        return costsOfSending;
    }

    public void setCostsOfSending(){
        this.costsOfSending = costsOfSending;
    }

    public Boolean getIsFire(){
        return isFire;
    }

    public void setIsFire(){
        this.isFire = isFire;
    }
}
