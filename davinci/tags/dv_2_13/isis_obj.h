typedef struct isis_kwd ISISKwd; 
typedef ISISKwd * ISISKwdPtr;

struct isis_kwd {
  char * keyword;
  int misc_seq;
  char * value;
  ISISKwdPtr next;
};

typedef struct isis_group ISISGrp;
typedef ISISGrp * ISISGrpPtr;

struct isis_group {
  char * group_name;
  int sequence;
  ISISGrpPtr parentgrp;
  ISISGrpPtr firstchildgrp;
  ISISGrpPtr nextsibgrp;
  ISISKwdPtr firstkwd;
};

typedef struct isis_object ISISObj;
typedef ISISObj * ISISObjPtr;

struct isis_object {
  char * objname;
  int sequence;
  int record_offset;
  ISISGrpPtr firstgroup;
  ISISKwdPtr firstkwd;
  ISISObjPtr nextobj;
};
