import java.util.Random;

enum Turn {
    MEN,
    WOMEN
}
// 
class Bathroom {
    // shared variables/resources between threads need to be protected
    protected int menInside = 0;
    protected int womenInside = 0;
    // unlike rw problem, we implement fairness, which means keeping track
    // of how many threads are currently using the resources and how many are 
    // waiting to use
    protected int waitingMen = 0;
    protected int waitingWomen = 0;

    protected int turn; // 1 for W, 0 for M

    public synchronized void manEnter(){
        waitingMen++;
        while (womenInside > 0 || (turn == 1 && waitingWomen>0)){
            try {
                wait();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt(); 
            }
        }
        waitingMen--;
        menInside++;
    }

    public synchronized void manExit(){
        menInside--;
        if (menInside == 0) {
            turn = 1;
            notifyAll();
        }
        
    
    }

    public synchronized void womanEnter(){
        waitingWomen++;
        while (menInside > 0 || (turn == 0 && waitingMen>0)){
            try {
                waitingWomen++;
                wait();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt(); 
            }
        }
        waitingWomen--;
        womenInside++;
    }

    public synchronized void womanExit(){
        womenInside--;
        if(womenInside==0){
            turn = 0;
            notifyAll();
        }
        
    }
}
class Man extends Thread {
    private final int id;
    private final Bathroom bathroom;
    private final Random rand = new Random();

    public Man(int id, Bathroom bathroom) {
        this.id = id;
        this.bathroom = bathroom;
    }

    public void run() {
        while (true) {
            try {
                // Step 1: work outside
                Thread.sleep(rand.nextInt(3000) + 1000);

                // Step 2: request entry
                System.out.println("Man " + id + " wants to enter");
                bathroom.manEnter();
                System.out.println("Man " + id + " enters bathroom");

                // Step 3: use bathroom
                Thread.sleep(rand.nextInt(2000) + 500);

                // Step 4: exit
                System.out.println("Man " + id + " leaving bathroom");
                bathroom.manExit();

            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return;
            }
        }
    }
}

class Woman extends Thread {
    private final int id;
    private final Bathroom bathroom;
    private final Random rand = new Random();

    public Woman(int id, Bathroom bathroom) {
        this.id = id;
        this.bathroom = bathroom;
    }

    public void run() {
        while (true) {
            try {
                Thread.sleep(rand.nextInt(3000) + 1000);

                System.out.println("Woman " + id + " wants to enter");
                bathroom.womanEnter();
                System.out.println("Woman " + id + " enters bathroom");

                Thread.sleep(rand.nextInt(2000) + 500);

                System.out.println("Woman " + id + " leaving bathroom");
                bathroom.womanExit();

            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return;
            }
        }
    }
}

class unisexBathroom {
    public static void main(String[] args) {
        final int NUM_MEN = 5;
        final int NUM_WOMEN = 5;

        Bathroom bathroom = new Bathroom();  // your monitor class

        // Create men threads
        for (int i = 0; i < NUM_MEN; i++) {
            new Man(i, bathroom).start();
        }

        // Create women threads
        for (int i = 0; i < NUM_WOMEN; i++) {
            new Woman(i, bathroom).start();
        }

        // Threads run forever; no join needed
    }
}
