// Usgage :  ROOT> .L tdc64a_test.C
//           ROOT> run_tdc64a(1000) //for 1000 events 

#include <unistd.h>
#include <stdio.h>

int run_tdc64a(int Nevent = 1000) {
  // local variables
  int sid = 1;  	               	// TDC64A USB3 SID
  
  // TDC setting
  int range = 16;                        // dynamic range (1/2/4/8/16/32/64 us)
  int dly_com = 0;                      // delay for common start, 0 ~ 4000 ns
  int dly_ch = 100;                     // delay for channel, 0 ~ 4000 ns
  int mask[64] = {1,1,1,1,1,1,0,0,      // TDC input mask, if set 1 input is ignored.
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,1,1,1,1,1,1,
                  1,1,1,1,1,1,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,1,1,1,1,1,1};

  // DAQ variable
  int tdc_data[1024];                    // TDC data array
  int hit[1024];                         // hit data array
  int ch[1024];                          // channel data array
  int evtn;      	                 // event number counter
  int trgn;                              // trigger number
  int data_size;                         // data size;              
  char filename[256];                    // data filename
  int nbins = 4096;                      // number of bins in us
  FILE *fp;
  int i;
  std::vector<TH1F*> h(64, nullptr);

  // set data filename
  sprintf(filename, "tdc64a.txt");

  // define some histograms
  c1 = new TCanvas("c1", "KFADC", 1500, 1000);
//  c1->Divide(8 ,8);
//  c1->Divide(4 ,4);
  c1->Divide(8 ,8);

// define histogram, adjust # of bins and range, bin = 1ns for example
  for (int i = 0; i < 64; ++i) {
    TString name  = Form("hist%d", i + 1);
    TString title = Form("Ch%d",   i + 1);
    h[i] = new TH1F(name, title, range * nbins, 0, range * 100000.0);
    h[i]->Reset(); // reset histogram
  }

  // Loading TDC64A lib.
  gSystem->Load("libNoticeTDC64AROOT.so");		   

  // define NKTCB class
  NKTDC64A *tdc = new NKTDC64A;
  
  // Initialize Libusb library, this one must be called once(either TCB side or FADC side)
  tdc->USB3Init(0);
  
  // open TCB 
  tdc->TDC64Aopen(sid, 0);
  
  // reset 
  tdc->TDC64Areset(sid); 

  // set dynamic range
  tdc->TDC64Awrite_RANGE(sid, range);
  
  // set common start delay
  tdc->TDC64Awrite_DLYCOM(sid, dly_com);

  // set channel delay
  tdc->TDC64Awrite_DLYCH(sid, dly_ch);

  // set TDC mask
  tdc->TDC64Awrite_TDCMASK(sid, mask);

  // readback dynamic range
  printf("Dynamic range = %d us\n", tdc->TDC64Aread_RANGE(sid));

  // readback common start delay
  printf("COM delay = %d ns\n", tdc->TDC64Aread_DLYCOM(sid));

  // readback common start delay
  printf("Channel delay = %d ns\n", tdc->TDC64Aread_DLYCH(sid));

  // readback TDC mask
  tdc->TDC64Aread_TDCMASK(sid, mask);
  printf("TDC mask = ");
  for (i = 0; i < 64; i++)
    printf("%d ", mask[i]);
  printf("\n");

  // open data file
  fp = fopen(filename, "wt");

  // start DAQ
  tdc->TDC64Astart(sid);

  for (evtn = 0; evtn < Nevent; evtn++) {
    // check data size, if it is not 0, read data
    data_size = 0;
    while (!data_size)
      data_size = tdc->TDC64Aread_DATASIZE(sid);

    // read data
    trgn = tdc->TDC64Aread_DATA(sid, data_size, tdc_data, hit, ch);

    // fill histogram and save data
    for (i = 0; i < data_size; i++) {
      fprintf(fp, "%d %d %d %d %d\n", trgn, evtn, ch[i], hit[i], tdc_data[i]);
        
      int idx = ch[i] - 1;
      if (0 <= idx && idx < 64) {
        h[idx]->Fill(tdc_data[i]);
      }
//*/
    }

//    fprintf(fp, "---------------------------------------\n");

    printf("%d / %d is taken, trigger # = %d data_size = %d\n", evtn + 1, Nevent, trgn, data_size);

    for (int i = 0; i < 64; ++i) {
      c1->cd(i + 1);
      h[i]->Draw();
    }
//*/
    c1->Modified();
    c1->Update();
  }

  // reset 
  tdc->TDC64Areset(sid); 

  // close file
  fclose(fp);

  // close TDC 
  tdc->TDC64Aclose(sid);

  // exit Libusb
  tdc->USB3Exit(0);
  
  return 0;	
}



