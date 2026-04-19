//-----------------------------------------------------------------------------
//  Dec 28 2000 P.Murat: base class for STNTUPLE input module
//  TopDir syntax: host:directory , i.e. fcdfsgi2.fnal.gov:/cdf/data/cafdfc ,
//  network catalog not implemented yet
//-----------------------------------------------------------------------------
#include "TROOT.h"
#include "TEnv.h"
#include "TChain.h"
#include "TChainElement.h"
#include "TSystem.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TFile.h"
#include "TString.h"

#include <set>

#include "Stntuple/base/TCdf2Files.hh"
#include "Stntuple/base/TStnFileset.hh"
#include "Stntuple/base/TStnDataset.hh"
#include "Stntuple/loop/TSamCatalogServer.hh"

ClassImp(TSamCatalogServer)
//_____________________________________________________________________________
TSamCatalogServer::TSamCatalogServer(const char* Url, const char* Rsh,
				     int Print): 
TStnCatalogServer(Url,Rsh,Print)
{
  TString  prefix; // initializes to ""
  TString  cmd; 
  char     hostname[200]="";
  //-----------------------------------------------------------------------------
//  now can use $USER on caf
//-----------------------------------------------------------------------------
  if (strcmp(GetHost(),hostname) == 0) {
    prefix = Form("%s/scripts",GetFile());
  }
  else {
    prefix = Form("%s -l %s %s %s/scripts",fRshCommand.Data(),
		  gSystem->Getenv("USER"),GetHost(),GetFile());
  }

  //  fGetDataServerNameCommand = Form("%s/get_server_name"         ,prefix.Data());
  fGetNEventsCommand        = Form("%s/get_nevents"             ,prefix.Data());
  fGetListOfFilesCommand    = Form("%s/get_list_of_files"       ,prefix.Data());
  fGetListOfBadFilesCommand = Form("%s/get_list_of_bad_files"   ,prefix.Data());
  fGetListOfFilesetsCommand = Form("%s/get_list_of_filesets"    ,prefix.Data());
  fGetListOfDatasetsCommand = Form("%s/get_list_of_datasets"    ,prefix.Data());
  //  fGetKeyCommand            = Form("%s/get_key"                 ,prefix.Data());

  // What are we going to use as retrieval commands
  if (fPrintLevel > 0)
    printf(" DEBUG HTML: Retrieving data with: %s\n",prefix.Data());

  fAAAFilesHtml.Clear();
}

//_____________________________________________________________________________
TSamCatalogServer::~TSamCatalogServer() {
  // destructor: module owns its histograms
}


//_____________________________________________________________________________
int TSamCatalogServer::FindDataset(const char* Book, const char* Dataset) {

  int nlines;
  int found = 0;
  TString cmd;
//-----------------------------------------------------------------------------
// query SAM 
//-----------------------------------------------------------------------------
  cmd = Form("source /cvmfs/mu2e.opensciencegrid.org/setupmu2e-art.sh; setup dhtools; samweb list-definitions --group=mu2e | grep %s | grep .stn$ | grep %s ",Book,Dataset);
  cmd += " | wc -l";

  if (fPrintLevel > 0) printf("TSamCatalogServer::%s cmd= %s\n", __func__, cmd.Data());

  FILE* pipe = gSystem->OpenPipe(cmd.Data(),"r");
  fscanf(pipe,"%i",&nlines);
  
  gSystem->ClosePipe(pipe);
  
  found = (nlines > 0);
  if (fPrintLevel > 0) printf("--> Found = %i\n", found);
  return found;
}

//_____________________________________________________________________________
int TSamCatalogServer::LoadSamDBFromHtml(const char* Book, 
						 const char* Dataset) {

  TString cmd;
  char buf[1000];
  FILE* pipe;

  fAAAFilesHtml.Delete();

  cmd = Form("wget %s/%s/%s/AAA_FILES.html  -o /dev/null -O /dev/stdout",
	     GetUrl(),Book,Dataset);
  if (fPrintLevel > 0) printf("LoadSamDBFromHtml cmd= %s\n",cmd.Data());

  pipe = gSystem->OpenPipe(cmd.Data(),"r");

  int nhtml = 0;
  while (fgets(buf,1000,pipe)) { 
    if ( buf[0] != '#' && buf[0] != '<' ) {
      fAAAFilesHtml.Add(new TObjString(buf));
      if (fPrintLevel > 50) printf("%s\n",buf);
      nhtml++;
    }
  }
  gSystem->ClosePipe(pipe);
  if (fPrintLevel > 0) printf("LoadSamDBFromHtml read %d lines\n",nhtml);

  // read the AAA_FILES.txt to make sure the html
  // is consistent with the standard catalog
  cmd = Form("wget %s/%s/%s/AAA_FILES.txt  -o /dev/null -O /dev/stdout",
	     GetUrl(),Book,Dataset);
  if (fPrintLevel > 0) printf("LoadSamDBFromHtml cmd= %s\n",cmd.Data());

  pipe = gSystem->OpenPipe(cmd.Data(),"r");
  
  int ntxt = 0;
  while (fgets(buf,1000,pipe)) { 
    if ( buf[0] != '#') ntxt++;
  }
  gSystem->ClosePipe(pipe);
  if (fPrintLevel > 0) printf("LoadSamDBFromHtml read %d lines\n",ntxt);

  if(ntxt!=nhtml) {
    printf("TSamCatalogServer: Warning - AAA_FILES.html is inconsistent with ");
    printf("      AAA_FILES.txt, (%d vs %d), use the txt file and Oracle db\n",
	   nhtml,ntxt);
    fAAAFilesHtml.Delete();
    return 1;
  }
  
  return 0;
}

//_____________________________________________________________________________
int TSamCatalogServer::AddFiles(TChain*     Chain  , 
				 const char* Book   ,
				 const char* Dataset, 
				 const char* Fileset,
				 Int_t       Run1   ,
				 Int_t       Run2   ) 
{
  char     buf[1000], fs[200], fn[200];
  char     date[50] , ctime[50];
  float    size;
  int      nevents,  lorun, loevt, hirun, hievt, within_the_range;
  TString  cmd, s_file;
//-----------------------------------------------------------------------------
// now get list of files
//-----------------------------------------------------------------------------
  cmd = Form("wget %s/%s/%s/AAA_FILES.txt  -o /dev/null -O /dev/stdout",
	     GetUrl(),Book,Dataset);

  if ((Fileset == 0) && (strcmp(Fileset,"") != 0)) {
    cmd += Form(" | awk '{if($1==\"%s\") print $0}'",Fileset);
  }
//-----------------------------------------------------------------------------
// skip empty lines, comments ('#') and html ('<')
//-----------------------------------------------------------------------------
  cmd += " | awk '{x = substr($0,1,1); if ((length($0) > 0) && (x != \"#\") && (x != \"<\")) print $0}'";

  FILE* pipe = gSystem->OpenPipe(cmd.Data(),"r");
//-----------------------------------------------------------------------------
// read output form a pipe line by line, skip comment lines
//-----------------------------------------------------------------------------
  while (fgets(buf,1000,pipe)) { 
    if ( buf[0] == '#')                                     goto NEXT_LINE; 

    sscanf(buf,"%s %s %f %s %s %i %i %i %i %i", 
	   fs,fn,&size,date,ctime,&nevents,&lorun,&loevt,&hirun,&hievt);

    if ((lorun > Run2) || (hirun < Run1)) within_the_range = 0;
    else                                  within_the_range = 1;

    if (within_the_range) {
//-----------------------------------------------------------------------------
//  time to add a file - make sure we're not adding the same file 2 times
//-----------------------------------------------------------------------------
      TObjArray* list_of_files = Chain->GetListOfFiles();

      TObjArrayIter it(list_of_files);
      TChainElement* found;

      while ((found = (TChainElement*) it.Next())) {
	if (strcmp(fn,found->GetTitle()) == 0) break;
      }
    
      if (! found) Chain->AddFile(fn,nevents);
    }
  NEXT_LINE:;
  }
  gSystem->ClosePipe(pipe);

  return 0;
}

//-----------------------------------------------------------------------------
// bad files are stored in 'bad_files.html' file
//-----------------------------------------------------------------------------
int TSamCatalogServer::InitListOfBadFiles(TStnDataset* Dataset) {

//   char     buf[1000], fn[1000];
// 
//   TString  cmd;
// //-----------------------------------------------------------------------------
// // check if already initialized
// //-----------------------------------------------------------------------------
//   if (Dataset->DoneBadFiles() != 0) return 0;
// //-----------------------------------------------------------------------------
// // now get list of files
// //-----------------------------------------------------------------------------
//   cmd = Form("wget %s/%s/%s/bad_files.html -o /dev/null -O /dev/stdout",
// 	     GetUrl(),Dataset->GetBook(),Dataset->GetName());
// 
//   if (fPrintLevel > 0) {
//     printf(" InitListOfBadFiles: Retrieving data with: %s\n",cmd.Data());
//   }
//     
//   FILE* pipe = gSystem->OpenPipe(cmd.Data(),"r");
// //-----------------------------------------------------------------------------
// // read output form a pipe line by line, skip comment lines
// //-----------------------------------------------------------------------------
//   while (fgets(buf,1000,pipe)) { 
//     if ( buf[0] == '#')                                     goto NEXT_LINE; 
// 
//     sscanf(buf,"%s",fn);
//     Dataset->GetListOfBadFiles()->Add(new TObjString(fn));
// 
//   NEXT_LINE:;
//   }
// 
//   gSystem->ClosePipe(pipe);
//   Dataset->SetDoneBadFiles();

  return 0;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
int TSamCatalogServer::InitListOfFilesets(TStnDataset* Dataset, 
					  const char*  Fileset,
					  const char*  File,
					  int          MinRun,
					  int          MaxRun,
					  TObjArray*   ListOfFilesets,
					  TObjArray*   ListOfFiles) 
{
  TString        grep_filesets;
  // TStnFileset*   fset;
  TString        cmd;
  char           buf[1000], fs[200]; // , fn[200];
  TString        s_fileset ; // , s_file;
  // char           date[100], time[100];
  //  int            nevents,  lorun, loevt, hirun, hievt, within_the_range;
  // float          size;
//-----------------------------------------------------------------------------
// read AAA_CATALOG.html
// skip comment, html and empty lines, 
//-----------------------------------------------------------------------------
  // TString sam_defname = Form("nts.mu2e.%s.%s.stn",Dataset->GetName(),Dataset->GetBook());
  TString sam_defname = Form("%s.%s.stn",Dataset->GetName(),Dataset->GetBook()); // should be nts.<user/mu2e>.<name>.<book>.stn
  cmd = Form("setup dhtools; DATASET=`samweb list-definitions --group=mu2e | grep %s | grep ^nts. | grep .stn$ | grep %s | tail -n 1`; ",Dataset->GetBook(),Dataset->GetName());
  // cmd += Form("samweb list-file-locations --dimensions \"dh.dataset ${DATASET}\""); //,sam_defname.Data());
  cmd += Form("setup mu2efiletools; mu2eDatasetFileList ${DATASET}"); //,sam_defname.Data());

  if (fPrintLevel > 0) {
    printf(" DEBUG FILESETS: Retrieving data with: %s\n",cmd.Data());
  }
//-----------------------------------------------------------------------------
// these are used only to make comparisons better readable...in the source code
//-----------------------------------------------------------------------------
  s_fileset  = Fileset;
  // s_file     = File;
    
  ListOfFilesets->Add(new TObjString("InitListOfFilesets_fake_fileset sam ./")); 
  
  FILE* pipe = gSystem->OpenPipe(cmd.Data(),"r");
  std::set<TString> found_files;
    
  while (fgets(buf,1000,pipe)) {
    sscanf(buf,"%s",fs);
//-----------------------------------------------------------------------------
// if s_fileset != "" only one fileset has been requested
//-----------------------------------------------------------------------------
    if (fPrintLevel > 0) {
      printf(" DEBUG 1FS: fileset: %s -> %s\n",s_fileset.Data(),fs);
    }
    // strcat(buf, " sam");
    if (fPrintLevel > 0) {
      printf(" DEBUG 1FS: fs = %s, buf = %s\n", fs, buf);
    }
    TString name(buf);
    name.ReplaceAll("enstore:", "");
    name.ReplaceAll(":", "");
    name.ReplaceAll("\t", "");
    if(found_files.contains(name)) {
      if(fPrintLevel > 0) printf("TSamCatalogServer::%s: Duplicate file found! Name = %s\n", __func__, name.Data());
      continue;
    }
    found_files.insert(name);
    ListOfFiles->Add(new TObjString(buf)); 
  }
  gSystem->ClosePipe(pipe);
  if (fPrintLevel > 0) {
    printf(" DEBUG FILESETS: fileset: %s found %zu files\n",s_fileset.Data(), found_files.size());
  }
  return 0;
}

//-----------------------------------------------------------------------------
// assuming key uniquely identifies the line
// the assumed line format  '# :key: value' (the key is surrounded by two ':' characters),
// such that the key is the second word, 
// Key is expected to be not case-sensitive
//-----------------------------------------------------------------------------
int TSamCatalogServer::GetDatasetKey(const char* Book, 
				      const char* Dataset,
				      const char* Key,
				      char*       Buffer) {
  TString cmd;

  cmd = Form("wget %s/%s/%s/AAA_CATALOG.html -o /dev/null -O /dev/stdout",
	     GetUrl(),Book,Dataset);
  cmd += Form(" | grep :%s: | awk -v key=:%s: '{if (toupper($2) == toupper(key)) print $3}'",Key,Key);

  if (fPrintLevel > 0)
    printf(" DEBUG DB KEY: Retrieving data with command: %s\n",cmd.Data());

  FILE* pipe = gSystem->OpenPipe(cmd.Data(),"r");
  fgets(Buffer,1000,pipe);
  gSystem->ClosePipe(pipe);

  return 0;
}

//_____________________________________________________________________________
int TSamCatalogServer::InitChain(TChain*     Chain, 
				 const char* Book,
				 const char* Dataset, 
				 const char* Fileset,
				 const char* File   ,
				 Int_t       Run1   ,
				 Int_t       Run2   ) {
//-----------------------------------------------------------------------------
// obsolete, use TSamCatalogServer::InitDataset
//-----------------------------------------------------------------------------
  Error("InitChain","Obsolete, read manual and use TSamCatalogServer::InitDataset");
  return -1;
}


//-----------------------------------------------------------------------------
// 'Fileset' may be a comma-separated list, for example, '000001,000002,000003'
// so far, query all files, assume one fileset
//-----------------------------------------------------------------------------
int TSamCatalogServer::InitDataset(TStnDataset*     Dataset,
				   const char*      Book   ,
				   const char*      Name   ,
				   const char*      Fileset,
				   const char*      File   ,
				   Int_t            MinRun ,
				   Int_t            MaxRun ,
				   const char*      Type   )
{
  FILE*        pipe;
  int          time, lorun, hirun, rmin, rmax, nev, loevt, hievt, status;
  int          n_filesets, mc_flag/*, add_file*/;
  float        size;
  char         buf[10000], date[1000], ctime[2000], fn[2000], fs[3000], fstemp[1000];
  char         full_name[3000], directory[200], server[200], pnfs_path[1000];
  const char   *line, *book, *dset; //, *dir;
  TObjArray    *list_of_filesets;
  TObjString   *ostr;
  TStnFileset  *fileset/*, *fset*/;
  TString      cmd;
  TString      s_file;
//-----------------------------------------------------------------------------
  if (strcmp(Book,"") != 0) {
    // inititalize dataset, otherwise assume it to be already initialized
    int rc = Dataset->Init(Book,Name,MinRun,MaxRun,Type);
    if (rc < 0) return rc;
  }

  book             = Dataset->GetBook();
  dset             = Dataset->GetName();
  s_file           = File;
//-----------------------------------------------------------------------------
// this is a cataloged dataset, verify AAA_CATALOG.html
//-----------------------------------------------------------------------------
  int found = FindDataset(book,dset);
  if (! found) return -1;

//-----------------------------------------------------------------------------
//  dataset is found , initalize metadata part of the dataset, so far all we 
//  need is a list of files with the corresponding "URL-type" prefixes
//  step 1: read all of the filesets at once - one transaction is better!
//-----------------------------------------------------------------------------
  list_of_filesets = Dataset->GetListOfFilesets();
  rmin             = Dataset->GetMinRunNumber  ();
  rmax             = Dataset->GetMaxRunNumber  ();
//-----------------------------------------------------------------------------
// retrieve MC type (flag: 'mc_flag' , long - for uniqueness), 
// returned value - integer
//-----------------------------------------------------------------------------
  buf[0] = 0;
  GetDatasetKey(book,dset,"MC_FLAG",buf);

  if (strlen(buf) > 0) {
    sscanf(buf,"%i",&mc_flag);
    Dataset->SetMcFlag(mc_flag);
  }
//-----------------------------------------------------------------------------
// retrieve MC process code 'PROCESS_CODE' and the number of generated events
// returned value - integer
//-----------------------------------------------------------------------------
  buf[0] = 0;
  GetDatasetKey(book,dset,"PROCESS_CODE",buf);

  if (strlen(buf) > 0) {
    int code;
    sscanf(buf,"%i",&code);
    Dataset->SetMCProcessCode(code);
  }

  buf[0] = 0;
  GetDatasetKey(book,dset,"NGENERATED",buf);

  if (strlen(buf) > 0) {
    long int n;
    sscanf(buf,"%li",&n);
    Dataset->SetNGenEvents(n);
  }
//-----------------------------------------------------------------------------
// retrieve list of bad files - hopefully short - do it only once - so far can
// do it multiple times
// kludge: allow to do reading multiple times for empty list
//-----------------------------------------------------------------------------
  InitListOfBadFiles(Dataset);
//-----------------------------------------------------------------------------
// retrieve list of filesets and also files in non-Oracle filesets...
//-----------------------------------------------------------------------------
  TObjArray filesets(100);
  TObjArray files   (1000);

  InitListOfFilesets(Dataset,Fileset,File,MinRun,MaxRun,&filesets,&files);

  n_filesets  = filesets.GetEntries();
  if (fPrintLevel > 0) printf(" DEBUG inDS: N(filesets) = %i\n", n_filesets);
  //  int n_files = files.GetEntriesFast();
//-----------------------------------------------------------------------------
// loop again over the fileset definition lines and parse the information
// only the files corresponding to the requested filesets are stored. 
// Loop over the filesets - some may be Oracle
//-----------------------------------------------------------------------------
  for (int i=0; i<n_filesets; i++) {
    line = (char*) ((TObjString*) filesets.At(i))->String().Data();
    sscanf(line,"%s %s %s %i %i %i %i %i",fs,server,directory,&nev,&lorun,&loevt,&hirun,&hievt);
//-----------------------------------------------------------------------------
//  new fileset, make sure we are not adding it twice - loop over the filesets
//-----------------------------------------------------------------------------
    if (fPrintLevel > 0) {
      printf(" DEBUG inDS: unparsed line: %s\n", line);
      printf(" DEBUG inDS: line: %s at %s in %s\n",fs,server,directory);
    }

    TString srv = server;
//-----------------------------------------------------------------------------
// ROOT4 and ROOT5 handle URL's in a different fascion:
// ROTO4 wants xxx.xxx.xxx/directory
// ROOT5 wants xxx.xxx.xxx//directory
// directory[0] is always '/', depending on whether a particular catalog has 
// been created for ROOT v4 ot ROOT v5, the second character may be either 
// '/', in which case notheng need to be done, or not, in which case one more
// '/' character needs to be added
//-----------------------------------------------------------------------------
    if ((gROOT->GetVersionInt() > 50000) && (directory[1] != '/')) srv += "/";
    srv   += directory;

    fileset = (TStnFileset*) list_of_filesets->FindObject(fs);
    if (fileset == 0) {
      fileset = new TStnFileset(fs,srv,0,nev,lorun,hirun);
      list_of_filesets->Add(fileset);
    }
    else {
      if (fileset->GetDataServer() == 0) {
	fileset->Set(srv,0,nev,lorun,hirun);
      }
      else {
	Error("InitDataset",Form("attempt to reinitialize fileset %s",fs));
	goto NEXT_FILESET;
      }
    }
//-----------------------------------------------------------------------------
// run range is OK by selection
//-----------------------------------------------------------------------------
    if (strcmp(server,"dfc") == 0) {
//-----------------------------------------------------------------------------
// query Oracle, directory="book:fileset"
//-----------------------------------------------------------------------------
      TString ss = directory;
      TString s1 = ss(0,ss.Index(":"));
      TString s2 = ss(ss.Index(":")+1,ss.Length());

      cmd  = Form("sqlplus cdf_reader/reader@%s <<EOF | ",GetOracleServer());
      cmd += Form("grep -s %s\n",s2.Data());
      cmd += Form("set linesize 1000;\n");
      cmd += Form("select fileset_name, file_name, file_size, ");
      cmd += Form("creation_time, ");
      cmd += Form("event_count, low_run, low_event, high_run, high_event ");
      cmd += Form("from %s.cdf2_files where ",s1.Data());
      cmd += Form("fileset_name='%s' ",s2.Data());
      cmd += Form("and low_run  <= %i and high_run >= %i",rmax,rmin);
      cmd += Form(";\nEOF\n");
      
      if (fPrintLevel > 0)
	printf(" DEBUG inDS SQL: Retrieving data with: %s\n",cmd.Data());      
      pipe = gSystem->OpenPipe(cmd.Data(),"r");
      while (fscanf(pipe,"%s %s %f %i %i %i %i %i %i",
  		    fs,fn,&size,&time,&nev,&lorun,&loevt,&hirun,&hievt)!=EOF) {
	if (fPrintLevel > 20) {
	  printf("Result of DFC query: %s %s %f %i %i %i %i %i %i\n",
		 fs,fn,size,time,nev,lorun,loevt,hirun,hievt);
	}
//-----------------------------------------------------------------------------
// so far list of bad files is kept locally ...
// make sure we're not adding a bad file
//-----------------------------------------------------------------------------
	if (Dataset->GetListOfBadFiles()->FindObject(fn) != 0) status = -1;
	else                                                   status =  0;
//-----------------------------------------------------------------------------
// form DCache name for this file and add it to the chain, handle the case when 
// a file has been requested by name explicitly
// files cataloged in DFC are read through general DCACHE pool (cdfdca)
//-----------------------------------------------------------------------------
	if ((s_file == "") || (s_file == fn)) {
	  const char* path = 0;
	  GetDCacheFileName("cdfdca.fnal.gov",path,fs,fn,full_name);
	  Dataset->AddFile(full_name,fs,size,nev,loevt,lorun,hievt,hirun,status);
	}
      }
      gSystem->ClosePipe(pipe);
    }


    else if (strcmp(server,"sam") == 0 && fAAAFilesHtml.GetEntries()>0 && fCafName=="fnal") {
//-----------------------------------------------------------------------------
//  AAA_FILES.html exists and is OK, use it for locations
//-----------------------------------------------------------------------------

      for(int ifile=0; ifile<fAAAFilesHtml.GetEntries(); ifile++) {
	const char* str = 
	  ((TObjString*)fAAAFilesHtml.At(ifile))->String().Data();
	sscanf(str,"%s %s %f %s %s %i %i %i %i %i %s",
	       fstemp,fn,&size,date,ctime,&nev,
	       &lorun,&loevt,&hirun,&hievt,pnfs_path);
	if(strstr(fs,fstemp)!=0) {
	  if (fPrintLevel > 20)
	    printf("From html file: %s %s %f %i %i %i %i %i %s\n",
		   fs,fn,size,nev,lorun,loevt,hirun,hievt,pnfs_path);
	  
	  if (Dataset->GetListOfBadFiles()->FindObject(fn) != 0) status = -1;
	  else                                                   status =  0;

	  if ((s_file == "") || (s_file == fn)) {
	    //	    const char* path = 0;
	    GetDCacheFileName("cdfdca.fnal.gov",pnfs_path,0,fn,full_name);
	    if(lorun<=rmax && hirun>=rmin) {
	      Dataset->AddFile(full_name,fs,size,nev,loevt,lorun,hievt,hirun,status);
	    }
	  } // endif parsing line
	} // endif right fileset
      } // end loop over files
    }
    else if (strcmp(server,"sam") == 0) {
      if(fPrintLevel > 0) printf("N(HTML entries) = %i\n", int(fAAAFilesHtml.GetEntries()));
      loevt = 0; lorun = 0; hievt = 0; hirun = 0; // FIXME: Get from catalog
      // Test replacing the query with a simple samweb call
      for(auto o : files) {
        TString line = TString(((TObjString*) o)->String());
        TString name = line;
        if(name.Contains(":")) { // samweb return style
          name = line(line.Index(":")+1,line.Sizeof()).Data();
          name.ReplaceAll(":", "");
          name.Replace(name.Index("\t"), 1, "/");
          size = TString(name(name.Index("\t")+1, name.Sizeof())).Atoi();
          name = name(0, name.Index("\t"));
        }
        name.ReplaceAll(" ", "");
        name.ReplaceAll("\t", "");
        name.ReplaceAll("\n", "");
        status = 0;
        if(fPrintLevel > 0) printf("Line = %s, Name = %s, size = %f\n", line.Data(),
                                   name.Data(), size);
        TFile* f = TFile::Open(name.Data(), "READ"); // FIXME: This info should be from the catalog
        if(!f) printf("TSamCatalogServer::%s: Failed to open file %s!\n", __func__, name.Data());
        else {
          TTree* tree = (TTree*) f->Get("STNTUPLE");
          nev         = int(tree->GetEntries());
          f->Close();
          delete f;
          Dataset->AddFile(name.Data(),fs,size,nev,loevt,lorun,hievt,hirun,status);
        }
      }

    }
    else if (strcmp(server,"sam") == 0) {
//-----------------------------------------------------------------------------
// query Oracle, directory = 
//-----------------------------------------------------------------------------
      TString ss = directory;
      TString s2 = ss+"."+fs;

      cmd  = Form("sqlplus cdf_reader/reader@%s <<EOF\n",GetOracleServer());
      cmd += Form("set linesize 500;\n");
      cmd += Form("set heading off;\n");
      cmd += Form("set feedback on;\n");
      cmd += Form("column location_id        format a40;\n");
      cmd += Form("column file_name          format a20;\n");
      cmd += Form("column proj_def_name      format a20;\n");
      cmd += Form("column full_path          format a80;\n");
      cmd += Form("column first_event_number format 999999999 ;\n");
      cmd += Form("column last_event_number  format 999999999 ;\n");
      cmd += Form("column proj_snap_version  format 99999 ;\n");

      cmd += Form("select smpjd.proj_def_name, smdf.file_name, smdf.file_size_in_bytes fsize, \n");
      cmd += Form(" 1 /* smdf.create_date */, \n");

      cmd += Form("smdf.event_count, \n");
      cmd += Form("(select min(smr.run_number) from data_files_runs smdfr, runs smr \n"); 
      cmd += Form(" where smdf.file_id = smdfr.file_id and smdfr.run_id = smr.run_id) low_run, \n");
      cmd += Form("smdf.first_event_number loevt, \n");

      cmd += Form("(select max(smr.run_number) from data_files_runs smdfr, runs smr \n"); 
      cmd += Form(" where smdf.file_id = smdfr.file_id and smdfr.run_id = smr.run_id) high_run, \n");
      cmd += Form("smdf.last_event_number hievt, sl.full_path \n");

      cmd += Form("from  project_definitions smpjd, project_snapshots smpjsnp, project_files smpjf, \n");
      cmd += Form("      data_files smdf, data_storage_locations sl, data_file_locations dfl \n");
      cmd += Form("where smpjd.proj_def_id = smpjsnp.proj_def_id and smpjsnp.proj_snap_id = smpjf.proj_snap_id and\n");
      cmd += Form("      smpjf.file_id = smdf.file_id and smpjd.proj_def_name = '%s' and \n",s2.Data());
      cmd += Form("      smdf.file_id = dfl.file_id and dfl.location_id = sl.location_id\n");
      if(fCafName=="cnaf") {  // location in remote station
	cmd += Form("      and 'station' in sl.location_type and instr(sl.full_path,'cnaf')>0\n");
      } else if(fCafName=="fnal") { // FNAL locations only, default
	cmd += Form("      and 'tape' in sl.location_type and substr(sl.full_path,1,20) = '/pnfs/cdfen/filesets'\n");
      }
      cmd += Form(";\nEOF\n");

      bool queryOk = false;
      int nTries = 0;

      while(!queryOk && nTries<10) {

	int filesRead = 0;
	int filesExpected = -1;

	pipe = gSystem->OpenPipe(cmd.Data(),"r");
	
	char allLines[50000] = "";
	char line[1000];
	while ( fgets(line,1000,pipe)!=NULL ){

	  strncat(allLines,line,1000);

	  if(strstr(line,"selected.")) { // this is summary line
	    if(strstr(line,"no ")) {
	      filesExpected = 0;
	    } else {
	      sscanf(line,"%i",&filesExpected);
	    }
	  } else if(strstr(line,s2.Data()) ) { // contains fileset name
	    sscanf(line,"%s %s %f %i %i %i %i %i %i %s",
		   fs,fn,&size,&time,&nev,&lorun,&loevt,&hirun,&hievt,pnfs_path);
	    
	    if (fPrintLevel > 20)
	      printf("Result of SAM query: %s %s %f %i %i %i %i %i %i\n",
		     fs,fn,size,time,nev,lorun,loevt,hirun,hievt);
	

//-----------------------------------------------------------------------------
// so far list of bad files is kept locally ...
// make sure we're not adding a bad file
//-----------------------------------------------------------------------------
	    if (Dataset->GetListOfBadFiles()->FindObject(fn) != 0) status = -1;
	    else                                                   status =  0;
//-----------------------------------------------------------------------------
// form DCache name for this file and add it to the chain, handle the case when 
// a file has been requested by name explicitly
// files cataloged in DFC are read through general DCACHE pool (cdfdca)
//-----------------------------------------------------------------------------
	    if ((s_file == "") || (s_file == fn)) {
	      //	      const char* path = 0;
	      if(fCafName=="cnaf") { 
		sprintf(full_name,"%s/%s",strstr(pnfs_path,":")+1,fn);
	      } else { // "fnal" and default
		GetDCacheFileName("cdfdca.fnal.gov",pnfs_path,0,fn,full_name);
	      }
	      if(lorun<=rmax && hirun>=rmin) {
		Dataset->AddFile(full_name,fs,size,nev,loevt,lorun,hievt,hirun,status);
	      }
	    }
	    filesRead++;

	  } else { // not a useful line - header or blank
	  } // endif parsing line
	  
	} // end while loop over query result lines
	gSystem->ClosePipe(pipe);
	
	if(filesRead>0 && filesExpected==filesRead) {
	  queryOk = true;
	} else {
	  printf("TSamCatalogServer: Error reading query\n");
	}

	if (fPrintLevel > 5 || !queryOk || nTries>0)
	  printf("After %s ntry %1i  files expected %i read %i\n",
		 s2.Data(),nTries,filesExpected,filesRead);
	if (fPrintLevel > 20 || (fPrintLevel>0 && !queryOk) ) {
	  printf("Sam query:\n");
	  printf("%s\n",cmd.Data());
	  printf("End query\n");
	  printf("Query output:\n");
	  printf("%s\n",allLines);
	  printf("End query output\n");
	}

	if(!queryOk) sleep(1);

	nTries++;
      } // end while loop to get correct query

      if(!queryOk) {
	printf("TSamCatalogServer: Error - failed to read sam filseset data after 10 tries.");
        return 1;
      }

    }
    else {
//-----------------------------------------------------------------------------
// 2020: currently: text-based catalog accessed via 'wget'
// list of files for this fileset already retrieved and stored in 'files'
// filesets from this list have already passed all the name checks
//-----------------------------------------------------------------------------
      TString prefix;

      TIter itt(&files);
      while ((ostr = (TObjString*) itt.Next())) {
	line = (char*) ostr->String().Data();
	
	sscanf(line,"%s %s %f %s %s %i %i %i %i %i", 
	       fs,fn,&size,date,ctime,&nev,&lorun,&loevt,&hirun,&hievt);
//-----------------------------------------------------------------------------
// so far list of bad files is kept locally ...
// make sure we're not adding a bad file
//-----------------------------------------------------------------------------
	status =  0;
	for(int i=0; i<Dataset->GetListOfBadFiles()->GetEntries(); i++) {
	  TObjString* str = (TObjString*)Dataset->GetListOfBadFiles()->At(i);
	  TString fns(fn);
	  if(fns.Index(str->String().Data()) >=0) status = -1;
	}
	if (strcmp(fs,fileset->GetName()) == 0) {
//-----------------------------------------------------------------------------
//  file belongs to the fileset, form URL-like name
//-----------------------------------------------------------------------------
	  TUrl* srv = fileset->GetDataServer();
	  if (strcmp(srv->GetProtocol(),"cdf_dcap") == 0)    {
//-----------------------------------------------------------------------------
// file in the analysis DCACHE pool 
//-----------------------------------------------------------------------------
	    TString s    = srv->GetFile();
	    TString path = s(1,s.Length()-1);

	    GetDCacheFileName(srv->GetHost(),path.Data(),0,fn,full_name);
	  }
	  else if (strcmp(srv->GetProtocol(),"cdf") == 0) {
//-----------------------------------------------------------------------------
// file in the analysis DCACHE pool 
//-----------------------------------------------------------------------------
	    TString s    = srv->GetFile();
	    TString path = s(1,s.Length()-1);

	    GetDCacheFileName(srv->GetHost(),path.Data(),0,fn,full_name);
	  }
	  else if (strcmp(srv->GetProtocol(),"bluearc") == 0) {
//-----------------------------------------------------------------------------
// BLUEARC: mount local root file - need a local host name
//-----------------------------------------------------------------------------
	    TString s    = srv->GetFile();
	    TString path = s(0,s.Length());

	    cmd = "hostname";
	    pipe = gSystem->OpenPipe(cmd.Data(),"r");
	
	    char hostname[1000];
	    fgets(hostname,1000,pipe);
	    gSystem->ClosePipe(pipe);
	    int len = strlen(hostname);
	    hostname[len-1] = 0;

	    //	    sprintf(full_name,"root://%s//%s/%s",hostname,path.Data(),fn);
	    sprintf(full_name,"//%s/%s",path.Data(),fn);
	  }
	  else if (strcmp(srv->GetProtocol(),"xroot") == 0) {
//-----------------------------------------------------------------------------
// XROOTD-based access
// at Fermilab, need VOMS certificate: kinit->kx509->voms-proxy-init
// xroot://fndca1.fnal.gov/pnfs/fnal.gov/usr/mu2e/scratch/users/murat/
//-----------------------------------------------------------------------------
 	    TString s    = srv->GetFile();
 	    TString path = s(0,s.Length());

// ####	    //	    sprintf(full_name,"root://%s//%s/%s",hostname,path.Data(),fn);
	    sprintf(full_name,"//%s/%s",path.Data(),fn);
	  }
	  else {
//-----------------------------------------------------------------------------
// anything else, so far: ROOTD
//-----------------------------------------------------------------------------
	    TString s    = srv->GetFile();
	    TString path = s(1,s.Length()-1);

	    sprintf(full_name,"root://%s//%s/%s",srv->GetHost(),path.Data(),fn);

	  }

	  Dataset->AddFile(full_name,fs,size,nev,loevt,lorun,hievt,hirun,status);
	}
      }
    }
  NEXT_FILESET:;
  }

  filesets.Delete();
  files.Delete();

  return 0;
}
//_____________________________________________________________________________
Int_t TSamCatalogServer::GetNEvents(const char* Book, 
				     const char* Dataset, 
				     const char* Fileset,
				     const char* File) 
{
  FILE*        pipe;
  int          /*time,*/ lorun, hirun, /*rmin, rmax,*/ nev, loevt, hievt/*, status*/;
  //  int          n_filesets/*, mc_flag*/;
  Int_t        /*nlines,*/ nevents;

  float        size;
  char         buf[10000], date[1000], ctime[2000], fn[2000], fs[1000];

  TString cmd;

  Error("GetNEvents","this works only in case of non-DFC file");

  nevents = 0;

  cmd = Form("wget -O /dev/stdout -o /dev/null %s/%s/%s/AAA_FILES.txt",
	     GetUrl(),Book,Dataset);

  if (Fileset) cmd += Form(" | awk '{if($1==\"%s\") print $0}' ",Fileset);
  if (File   ) cmd += Form(" | grep %s",File   );

  pipe = gSystem->OpenPipe(cmd.Data(),"r");
  while (fgets(buf,1000,pipe)) { 
//-----------------------------------------------------------------------------
// skip comment lines
//-----------------------------------------------------------------------------
    if ( buf[0] == '#')                                     goto NEXT_LINE; 

    sscanf(buf,"%s %s %f %s %s %i %i %i %i %i", 
	   fs,fn,&size,date,ctime,&nev,&lorun,&loevt,&hirun,&hievt);
    nevents = nev;

  NEXT_LINE:;
  }
  gSystem->ClosePipe(pipe);

  return nevents;
}
//_____________________________________________________________________________
Int_t TSamCatalogServer::GetNFiles(const char* Book, 
				   const char* Dataset, 
				   const char* Fileset)
{
  Int_t   n = -1;

  Error("GetNFiles","Not implemented yet");
  return n;
}

//_____________________________________________________________________________
Int_t TSamCatalogServer::GetNFilesets(const char* Book, const char* Dataset) {
  Int_t   n = -1;

  Error("GetNFilesets","Not implemented yet");
  return n;
}

//_____________________________________________________________________________
int TSamCatalogServer::GetRemoteServer(const char* Book     ,
				       const char* Dataset  ,
				       const char* Fileset  ,
				       char*       Server   ,
				       char*       RemoteDir)
{
  // returns  smth like Server="ncdf131.fnal.gov" and 
  // RemoteDir="/cdf/scratch/data131" - address of the directory 
  // where the fileset is located

  TString cmd; 

  cmd = Form("wget -O /dev/stdout -o /dev/null %s/%s/%s/AAA_CATALOG.html",
	     GetUrl(),Book,Dataset);

  cmd += Form(" | awk '{ if($1==\"%s\") print $1 \" \" $2}' ",Fileset);

  FILE* pipe = gSystem->OpenPipe(cmd,"r");
  fscanf(pipe,"%s %s",Server,RemoteDir);
  gSystem->ClosePipe(pipe);

  return 0;
}


