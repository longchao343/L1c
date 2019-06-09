/*
  Tests for the 1D DCT-based transforms.

 */
#include "config.h"


#define CK_FLOATING_DIG 20
#include <stdlib.h>
#include <stdio.h>
#include <math.h> //Constants
#include <check.h>
#include <cjson/cJSON.h>


/* Tolerances and things */
#include "test_constants.h"
/* To read in test data */
#include "json_utils.h"
#include "check_utils.h"
#include "l1c.h"

#define TOL_LARGE_DCT TOL_DOUBLE_SUPER

extern char* fullfile(char *base_path, char *name);
extern char *test_data_dir;

typedef struct DctData{
  l1c_int *pix_idx;
  double *MtEty_EMx_exp;
  double *MtEty_EMx_act;
  double *MtEty_act;
  double *MtEty_exp;
  double *EMx_act;
  double *EMx_exp;

  double *Mx_act;
  double *Mx_exp;
  double *Mty_act;
  double *Mty_exp;

  double *Ex_act;
  double *Ex_exp;
  double *Ety_act;
  double *Ety_exp;

  double *x_in;
  double *y_in;
  double *z_in;

  /* Transform is n by mtot. mtot = mrow*mcol.*/
  l1c_int mrow;
  l1c_int mcol;
  l1c_int mtot;
  l1c_int n;

  int setup_status;
  char fpath[256];

}DctData;

/* Global variable for all test cases.
 */

DctData *dctd;
l1c_AxFuns ax_funs;

/* We the tcase_add_checked_fixture method. setup() and teardown are called by
   the associated setup and teardown functions for each test case. This allows us
   to use a different file path for each.
 */
static void setup(DctData *dct_dat){
  cJSON *test_data_json;

  int setup_status=0;
  if (load_file_to_json(dct_dat->fpath, &test_data_json)){
    fprintf(stderr, "Error loading data in test_dct\n");
    ck_abort();
  }

  setup_status +=extract_json_double_array(test_data_json, "x_in", &dct_dat->x_in, &dct_dat->mtot);
  setup_status +=extract_json_double_array(test_data_json, "y_in", &dct_dat->y_in, &dct_dat->n);
  setup_status +=extract_json_double_array(test_data_json, "z_in", &dct_dat->z_in, &dct_dat->mtot);

  setup_status +=extract_json_double_array(test_data_json, "MtEt_EMx", &dct_dat->MtEty_EMx_exp, &dct_dat->mtot);

  setup_status +=extract_json_double_array(test_data_json, "MtEty", &dct_dat->MtEty_exp, &dct_dat->mtot);

  setup_status +=extract_json_double_array(test_data_json, "EMx", &dct_dat->EMx_exp, &dct_dat->n);

  setup_status +=extract_json_double_array(test_data_json, "Mx", &dct_dat->Mx_exp, &dct_dat->mtot);
  setup_status +=extract_json_double_array(test_data_json, "Mty", &dct_dat->Mty_exp, &dct_dat->mtot);
  setup_status +=extract_json_double_array(test_data_json, "Ex", &dct_dat->Ex_exp, &dct_dat->n);
  setup_status +=extract_json_double_array(test_data_json, "Ety", &dct_dat->Ety_exp, &dct_dat->mtot);

  setup_status +=extract_json_int_array(test_data_json, "pix_idx", &dct_dat->pix_idx, &dct_dat->n);
  setup_status +=extract_json_int(test_data_json, "mrow", &dct_dat->mrow);
  setup_status +=extract_json_int(test_data_json, "mcol", &dct_dat->mcol);

  if (setup_status){
    fprintf(stderr, "Error Loading json into program data in 'test_MtEty_large()'. Aborting\n");
    ck_abort();
  }
  ck_assert_int_eq(dct_dat->mrow * dct_dat->mcol, dct_dat->mtot);

  dct_dat->MtEty_EMx_act = l1c_malloc_double(dct_dat->mtot);
  dct_dat->MtEty_act = l1c_malloc_double(dct_dat->mtot);
  dct_dat->EMx_act = l1c_malloc_double(dct_dat->mtot);
  dct_dat->Mx_act = l1c_malloc_double(dct_dat->mtot);
  dct_dat->Mty_act = l1c_malloc_double(dct_dat->mtot);
  dct_dat->Ex_act = l1c_calloc_double(dct_dat->n);
  dct_dat->Ety_act = l1c_malloc_double(dct_dat->mtot);


  cJSON_Delete(test_data_json);
}

static void teardown(DctData *dct_dat){
  l1c_free_double(dct_dat->x_in);
  l1c_free_double(dct_dat->y_in);
  l1c_free_double(dct_dat->z_in);

  l1c_free_double(dct_dat->MtEty_EMx_exp);
  l1c_free_double(dct_dat->MtEty_exp);
  l1c_free_double(dct_dat->EMx_exp);
  l1c_free_double(dct_dat->Mx_exp);
  l1c_free_double(dct_dat->Mty_exp);
  l1c_free_double(dct_dat->Ex_exp);
  l1c_free_double(dct_dat->Ety_exp);

  l1c_free_double(dct_dat->MtEty_EMx_act);
  l1c_free_double(dct_dat->MtEty_act);
  l1c_free_double(dct_dat->EMx_act);
  l1c_free_double(dct_dat->Mx_act);
  l1c_free_double(dct_dat->Mty_act);
  l1c_free_double(dct_dat->Ex_act);
  l1c_free_double(dct_dat->Ety_act);

  free(dct_dat->pix_idx);

}

static void setup_dct2_square(void){
  dctd = malloc(sizeof(DctData));

  char *fpath_dct2_small = fullfile(test_data_dir, "dct2_small.json");

  sprintf(dctd->fpath, "%s", fpath_dct2_small);
  setup(dctd);

  free(fpath_dct2_small);
}

static void teardown_dct2_square(void){
  teardown(dctd);
  free(dctd);
}



START_TEST(test_dct2_only)
{
  l1c_int n = dctd->n;
  l1c_int mrow = dctd->mrow;
  l1c_int mcol = dctd->mcol;
  double alp_v = 0;
  double alp_h = 0;
  int status = 0;
  status = l1c_setup_dctTV_transforms(n, mrow, mcol, alp_v, alp_h,
                                      dctd->pix_idx, &ax_funs);
  ck_assert_int_eq(status, L1C_SUCCESS);

  ck_assert_int_eq(ax_funs.n, n);
  ck_assert_int_eq(ax_funs.p, mrow*mcol);
  ck_assert_int_eq(ax_funs.m, mrow*mcol);

  ck_assert_ptr_eq(ax_funs.data, NULL);

  // ax_funs.AtAx(dctd->x_in, dctd->MtEty_EMx_act);

  // ck_assert_double_array_eq_tol(dctd->mtot, dctd->MtEty_EMx_exp,
  //                               dctd->MtEty_EMx_act, TOL_DOUBLE_SUPER);

  ax_funs.destroy();

}
END_TEST


// START_TEST(test_dct2_MtEty)
// {
//   ax_funs.Aty(dctd->y_in, dctd->MtEty_act);

//   ck_assert_double_array_eq_tol(dctd->mtot, dctd->MtEty_exp,
//                                 dctd->MtEty_act, TOL_DOUBLE_SUPER);

// }
// END_TEST

// START_TEST(test_dct2_EMx)
// {
//   ax_funs.Ax(dctd->x_in, dctd->EMx_act);

//   ck_assert_double_array_eq_tol(dctd->n, dctd->EMx_exp,
//                                 dctd->EMx_act, TOL_DOUBLE_SUPER);

// }
// END_TEST

// START_TEST(test_dct2_Mx)
// {
//   ax_funs.Mx(dctd->x_in, dctd->Mx_act);

//   ck_assert_double_array_eq_tol(dctd->mtot, dctd->Mx_exp,
//                                 dctd->Mx_act, TOL_DOUBLE_SUPER);

// }
// END_TEST

// START_TEST(test_dct2_Mty)
// {
//   ax_funs.Mty(dctd->z_in, dctd->Mty_act);

//   ck_assert_double_array_eq_tol(dctd->mtot, dctd->Mty_exp,
//                                 dctd->Mty_act, TOL_DOUBLE_SUPER);

// }
// END_TEST

// START_TEST(test_dct2_Ex)
// {
//   ax_funs.Ex(dctd->x_in, dctd->Ex_act);

//   ck_assert_double_array_eq_tol(dctd->n, dctd->Ex_exp,
//                                 dctd->Ex_act, TOL_DOUBLE_SUPER);

// }
// END_TEST

// START_TEST(test_dct2_Ety)
// {
//   ax_funs.Ety(dctd->y_in, dctd->Ety_act);

//   ck_assert_double_array_eq_tol(dctd->mtot, dctd->Ety_exp,
//                                 dctd->Ety_act, TOL_DOUBLE_SUPER);

// }
// END_TEST


/* Check that the setup function returns an error code when we expect it to. */
START_TEST(test_input_errors)
{
  l1c_int status=0, n=5, mrow=10, mcol=10;
  double alp_v = 1.0;
  double alp_h = 1.0;
  l1c_int pix_idx[5] = {0, 3, 4, 6, 7};

  /* alp_v < 0 should throw an error. */
  alp_v = -1.0;
  status = l1c_setup_dctTV_transforms(n, mrow, mcol, alp_v, alp_h, pix_idx, &ax_funs);

  ck_assert_int_eq(status, L1C_INVALID_ARGUMENT);

  /* alp_h < 0 should throw an error. */
  alp_v = 1.0;
  alp_h = -1.0;
  status = l1c_setup_dctTV_transforms(n, mrow, mcol, alp_v, alp_h, pix_idx, &ax_funs);

  /* alp_v > 0, requires mrow>2, mcol>2   */
  alp_v = 1.0;
  alp_h = 0.0;
  mcol = 10;
  mrow = 1;
  status = l1c_setup_dctTV_transforms(n, mrow, mcol, alp_v, alp_h, pix_idx, &ax_funs);
  ck_assert_int_eq(status, L1C_INVALID_ARGUMENT);

  mrow = 10;
  mcol = 1;
  status = l1c_setup_dctTV_transforms(n, mrow, mcol, alp_v, alp_h, pix_idx, &ax_funs);
  ck_assert_int_eq(status, L1C_INVALID_ARGUMENT);


  /* alp_h > 0, requires mrow>2, mcol>2   */
  alp_v = 0.0;
  alp_h = 1.0;
  mcol = 10;
  mrow = 1;
  status = l1c_setup_dctTV_transforms(n, mrow, mcol, alp_v, alp_h, pix_idx, &ax_funs);
  ck_assert_int_eq(status, L1C_INVALID_ARGUMENT);

  mrow = 10;
  mcol = 1;
  status = l1c_setup_dctTV_transforms(n, mrow, mcol, alp_v, alp_h, pix_idx, &ax_funs);
  ck_assert_int_eq(status, L1C_INVALID_ARGUMENT);

}
END_TEST


/* Add all the test cases to our suite
 */
Suite *dctTV_suite(void)
{
  Suite *s;
  TCase *tc_errors, *tc_dct2_only;
  // TCase *tc_dct2_square, *tc_dct2_pure, *tc_dct2_tall, *tc_dct2_wide;
  // TCase  *tc_dct2_square;
  s = suite_create("dctTV");

  tc_errors = tcase_create("dct_tv_errors");
  tcase_add_test(tc_errors, test_input_errors);

  suite_add_tcase(s, tc_errors);


  tc_dct2_only = tcase_create("dctTV_dct2_only");
  tcase_add_checked_fixture(tc_dct2_only, setup_dct2_square, teardown_dct2_square);
  tcase_add_test(tc_dct2_only, test_dct2_only);

  suite_add_tcase(s, tc_dct2_only);

  // tc_dct2_square = tcase_create("dct2_small");
  // tcase_add_checked_fixture(tc_dct2_square, setup_square, teardown_square);
  // tcase_add_test(tc_dct2_square, test_dct2_MtEt_EMx);
  // tcase_add_test(tc_dct2_square, test_dct2_MtEty);
  // tcase_add_test(tc_dct2_square, test_dct2_EMx);
  // tcase_add_test(tc_dct2_square, test_dct2_Mx);
  // tcase_add_test(tc_dct2_square, test_dct2_Mty);
  // tcase_add_test(tc_dct2_square, test_dct2_Ex);
  // tcase_add_test(tc_dct2_square, test_dct2_Ety);


  /* The test check what happens when pix_idx is all ones, ie,
     the identitiy. That is, these are equivalent to just taking the dct/idct
     with not sampling.
  */



  return s;

}
