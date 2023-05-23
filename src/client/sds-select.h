/**
 * *** Copyright Notice ***
 * SDS - Scientific Data Services framework, Copyright (c) 2015, The Regents of the University of California, through Lawrence Berkeley National Laboratory (subject to receipt of any required approvals from the U.S. Dept. of Energy).  All rights reserved.
 * If you have questions about your rights to use or distribute this software, 
 * please contact Berkeley Lab's Technology Transfer Department at TTD@lbl.gov.
 * 
 * NOTICE.  This software was developed under funding from the 
 * U.S. Department of Energy.  As such, the U.S. Government has been granted 
 * for itself and others acting on its behalf a paid-up, nonexclusive, 
 * irrevocable, worldwide license in the Software to reproduce, prepare 
 * derivative works, and perform publicly and display publicly.  
 * Beginning five (5) years after the date permission to assert copyright is 
 * obtained from the U.S. Department of Energy, and subject to any subsequent 
 * five (5) year renewals, the U.S. Government is granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide license
 * in the Software to reproduce, prepare derivative works, distribute copies to
 * the public, perform publicly and display publicly, and to permit others to
 * do so.
 *
*/

/**
 *
 * Email questions to {dbin, sbyna, kwu}@lbl.gov
 * Scientific Data Management Research Group
 * Lawrence Berkeley National Laboratory
 *
*/
//Organized as a tree 
typedef union SDS_Range_boundary
{
   int   i;
   float f;
}SDS_Range_boundary;


typedef struct SDS_Equal{
  SDS_Range_boundary                  r_max;
  SDS_Bool                            value_space;
}SDS_Equal

typedef struct SDS_Range{
  SDS_Range_boundary                  *r_max;
  SDS_Range_boundary                  *r_min;
  SDS_Bool                             value_space;
}SDS_Range
  
typedef struct ConditionTree{
  int                    node_type;         // 0: leaf node (variable);  1: non-leaf node (for combination)
  int                    var_index;          // id information 
  SDS_Combine_type       combine_type;      // How to combine left and right children 
  struct ConditionTree  *left;
  struct ConditionTree  *right;
}ConditionTree;

typedef struct SDS_Select_operand{
  int            *var_index;
  int             length;
  ConditionTree  *tree;
}SDS_Select_operand;
