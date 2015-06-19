import java.net.*;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import java.io.BufferedReader;
import java.io.InputStreamReader;

public class recv {
    /* Configurables */
    public static final double WEIGHT = 0.5;
    public static final double PROBING_RATE = 0.3;
    /* -------------------- */
    
    /* CONSTANTS */
    public static final int E_THROUGHPUT = 0;
    public static final int PER = 1;
    public static final double rates[] = {1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 52};
    /* -------------------- */
    
    public static int current_Rate = 4;
    public static int prev_Rate = 0;
    public static int leader = 0;
    
    public static int FIXED_RATE = 0;

    public static long total_sent = 0;
    public static long total_received = 0;
    public static boolean probed = false;

    public static double rate_table[][] = new double[12][2];

    public static final String pkts_Sent = "iptables -t mangle --list -v | awk '{if ($3 == \"TTL\") print $ 1}'";
    public static final String reset = "iptables -t mangle -F";
    public static final String mangle = "iptables -t mangle -A OUTPUT -j TTL -d 224.0.67.67 --ttl-set ";
    public static final String fpath = "/sys/kernel/debug/ieee80211/phy0/ath9k/mcast_rate";
    
    public static String execute_shell_command(String command)
    {
        String output = new String();
        try {
            Process p = Runtime.getRuntime().exec(new String[]{"bash","-c",command});
            p.waitFor();
            BufferedReader buf = new BufferedReader(new InputStreamReader(p.getInputStream()));
            output = buf.readLine();

        } catch (Exception e) {}
        return output;
    }
    public static int count_sent()
    {
        int s = 0;
        try {
            s = (Integer.parseInt(execute_shell_command(pkts_Sent)));
        } catch(Exception e) {}
        
        execute_shell_command("iptables -t mangle -Z");
        return s;
    }
    
    public static void set_Rate(int rate)
    {
        if (FIXED_RATE != 0)
        {
            current_Rate = FIXED_RATE;
            rate = FIXED_RATE;
        }

        if(prev_Rate - current_Rate != 0)
        {   
            execute_shell_command(new String("echo " + rate + " > " + fpath));
            int ttl = (2*rate)+5;

            int write_value = ttl | (leader << 5);
            total_sent += count_sent();
            
            execute_shell_command(reset);
            execute_shell_command(new String(mangle + write_value));
             System.out.println(prev_Rate + " NEW Rate: " + current_Rate);
        }
    }

    public static void find_Rate()
    {
        /* Probing 
        Don't probe if previous turn was probed
        */
        if (Math.random() < PROBING_RATE)
        {
            /*for (int i = current_Rate + 1; i < 12; i++)
            {
                if (rate_table[i][E_THROUGHPUT] == 0)
                {
                    current_Rate = i;
                    break;
                }
            }
            */
            if (probed == false)
            {
                double chance = 0.75;
                for (int i = current_Rate - 2; i < current_Rate + 2; i++)
                {
                    if (i == current_Rate || i < 1 || i > 11)
                    {
                        continue;
                    }
                    if (Math.random() > chance)
                    {
                        current_Rate = i;
                        probed = true;
                        break;
                    }
                    chance -= 0.25;
                }
            }
        }
        else
        {
            probed = false;
            for (int i=0; i < 12; i++)
            {
                if (rate_table[i][E_THROUGHPUT] > rate_table[current_Rate][E_THROUGHPUT])
                {
                    current_Rate = i;
                }
            }
        }
        
        set_Rate(current_Rate);
    }

    public static void updateRateTable(int[] mcs_feedback)
    {
        int sent = count_sent();
        total_sent += sent;

        for (int i=0; i<sent; i++)
        {
            if (FIXED_RATE == 0)
                System.out.println(current_Rate + " ");
        }
        double per = 0;

        double lost = sent - mcs_feedback[current_Rate];

        if (sent < mcs_feedback[current_Rate])
        {
            total_sent += mcs_feedback[current_Rate] - sent;
        }

        //total_received += mcs_feedback[current_Rate];
        
        for (int i=0; i<12; i++)
        {
            total_received += mcs_feedback[i];
        }
        
        if (lost < sent && lost >= 0)
            per = lost/(double)sent;
        else 
            per = 0;

        rate_table[current_Rate][PER] = WEIGHT*rate_table[current_Rate][PER] 
            + ((double)(1 - WEIGHT))*(per);

        // System.out.println(mcs_feedback[current_Rate] + " / " + sent);

        rate_table[current_Rate][E_THROUGHPUT] = rates[current_Rate]*(1 - rate_table[current_Rate][PER]);
        // System.out.println(Arrays.deepToString(rate_table));
    }
    
    public static void main(String[] args)
    {
        Runtime.getRuntime().addShutdownHook(new Thread() {
            @Override
            public void run() {
                double error_rate = (double)(total_sent - total_received)/(double)(total_sent);
                
                System.out.println("Throughput: " + 8*(1-error_rate)*(total_sent*1500)/(60*1024*1024) + " Mbps");
                System.out.println("Error_Rate: " + 100*error_rate + " %");
                System.out.println("Exiting...");
            }
        });
        if (args.length > 0)
        {
            FIXED_RATE = Integer.parseInt(args[0]);
            System.out.println("Rate Fixed to " + FIXED_RATE);
        }

        try{
            // DatagramSocket serverSocket = new DatagramSocket(1532);
            MulticastSocket serverSocket = new MulticastSocket(1532);
            serverSocket.joinGroup(InetAddress.getByName("224.0.67.67"));
            byte[] receiveData = new byte[48];

            set_Rate(current_Rate);
            while(true)
            {
                DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
                serverSocket.receive(receivePacket);
                byte[] temp = receivePacket.getData();
                
                IntBuffer intBuf =
                    ByteBuffer.wrap(temp)
                    .order(ByteOrder.LITTLE_ENDIAN)
                    .asIntBuffer();
                    int[] mcs_feedback = new int[intBuf.remaining()];
                    intBuf.get(mcs_feedback);

                if (leader == 0 || mcs_feedback[0] == 999)
                {
                    int val = ByteBuffer.wrap(receivePacket.getAddress().getAddress()).getInt();
                    leader = (val & 0xFF);
                    set_Rate(current_Rate);
                }

                updateRateTable(mcs_feedback);
                prev_Rate = current_Rate;
                find_Rate();
           }
        } catch (Exception e){e.printStackTrace(); }
    }
}