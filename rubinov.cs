using System; 

public class Hello1
{
   public static void Main()
   {


//     string oneCase = "1 Master 112 Stat= 7013 Arm= 0000 Trig_in_mem= 000B Memcode= 1160 Currenttrig= 000A PADEtemp= 001C SIBtemp= 007F"; 
     string oneCase2 = "1 Master 112 0111 0000 0060 1C00 005E 0019 0032"; 
     string oneCase3 = "1 Master 101 ccd 0 81 5ab c20 2198 fe9";
     string multiCase = "2 Master 112 0111 0000 0060 1C00 005E 0019 0032 Slave 113 8551 0000 0060 1C00 005E 0018 0032"; 

     string[] tt; 
     GetStatus(out tt, oneCase3); 

     foreach (string s in tt)
       {
	 System.Console.WriteLine(s); 
       }
     GetStatus(out tt, multiCase); 
     foreach (string s in tt)
       {
	 System.Console.WriteLine(s); 
       }




   }
  public static int GetStatus(out string[] status, string t)
  {
    string[] n = new string[10];
    n[0] = "";
    n[1] = "";
    n[2] = "";
    n[3] = "st=";
    n[4] = "ARM=";
    n[5] = "t in mem=";
    n[6] = "err reg=";
    n[7] = "last trig=";
    n[8] = "Ptemp=";
    n[9] = "Stemp=";
    int lines = 0;
    int num_pade = 0;
    string[] tok = new string[1];

//    SW.WriteLine("status");

  //  string t = SR.ReadLine();
    lines++;
    if (t.ToUpper().Contains("MASTER") || t.ToUpper().Contains("SLAVE"))
      {
	string[] delim = new string[1];
	delim[0] = " ";
	tok = t.Split(delim, StringSplitOptions.RemoveEmptyEntries);
	num_pade = Convert.ToInt32(tok[0]);
      }

//    System.Console.WriteLine("Token List"); 

//    int fg = 0; 
//    foreach (string token in tok)
//      {

//	System.Console.WriteLine("{0},{1}", fg, token); 
//	fg++; 
//      }
//    System.Console.WriteLine("------------------------------------------"); 
    lines = num_pade;
    string[] s = new string[lines+1];
    if ((tok.Length < 9 * num_pade) || (num_pade == 0))
      {
	for (int i = 0; i < s.Length; i++)
	  {
	    s[i] = "error";
	  }
	if (num_pade == 0) { s = new string[1]; s[0] = "error, 0 PADE"; }
      }
    else
      {
	int j = 0;
	int k = 0;
	s[k] = "";
	for (int i = 0; i < tok.Length; i++)
	  {
	    int nCalc = j-9*k; 
	    string nCalcTok = tok[j];
//	    System.Console.WriteLine("Ncalc:{0}, nCalcTok:{1}", nCalc, nCalcTok); 
//	    System.Console.WriteLine("i:{0}, j:{1}, k:{2}, S:{3}", i,j,k,s[k]);          
	    if (j>0)
	      {
		s[k] += n[j-9*k]+tok[j] + " ";
	      }
	    j++;
	    if ((j-1) >= (9 * (k + 1))) { k++; s[k] = ""; }
	  }
      }
    status = s;
    return lines;
  }


}

