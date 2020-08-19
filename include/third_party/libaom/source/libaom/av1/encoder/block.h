/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#ifndef AOM_AV1_ENCODER_BLOCK_H_
#define AOM_AV1_ENCODER_BLOCK_H_

#include "av1/common/entropymv.h"
#include "av1/common/entropy.h"
#include "av1/common/enums.h"
#include "av1/common/mvref_common.h"

#include "av1/encoder/enc_enums.h"
#if !CONFIG_REALTIME_ONLY
#include "av1/encoder/partition_cnn_weights.h"
#endif

#include "av1/encoder/hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MC_FLOW_BSIZE_1D 16
#define MC_FLOW_NUM_PELS (MC_FLOW_BSIZE_1D * MC_FLOW_BSIZE_1D)
#define MAX_MC_FLOW_BLK_IN_SB (MAX_SB_SIZE / MC_FLOW_BSIZE_1D)
#define MAX_WINNER_MODE_COUNT_INTRA 3
#define MAX_WINNER_MODE_COUNT_INTER 1

// SuperblockEnc stores superblock level information used by the encoder for
// more efficient encoding.
typedef struct {
  // The maximum and minimum allowed partition size
  BLOCK_SIZE min_partition_size;
  BLOCK_SIZE max_partition_size;

  // Below are information gathered from tpl_model used to speed up the encoding
  // process.
  int tpl_data_count;
  int64_t tpl_inter_cost[MAX_MC_FLOW_BLK_IN_SB * MAX_MC_FLOW_BLK_IN_SB];
  int64_t tpl_intra_cost[MAX_MC_FLOW_BLK_IN_SB * MAX_MC_FLOW_BLK_IN_SB];
  int_mv tpl_mv[MAX_MC_FLOW_BLK_IN_SB * MAX_MC_FLOW_BLK_IN_SB]
               [INTER_REFS_PER_FRAME];
  int tpl_stride;
} SuperBlockEnc;

typedef struct {
  MB_MODE_INFO mbmi;
  RD_STATS rd_cost;
  int64_t rd;
  int rate_y;
  int rate_uv;
  uint8_t color_index_map[64 * 64];
  THR_MODES mode_index;
} WinnerModeStats;

typedef struct {
  unsigned int sse;
  int sum;
  unsigned int var;
} DIFF;

enum {
  NO_TRELLIS_OPT,          // No trellis optimization
  FULL_TRELLIS_OPT,        // Trellis optimization in all stages
  FINAL_PASS_TRELLIS_OPT,  // Trellis optimization in only the final encode pass
  NO_ESTIMATE_YRD_TRELLIS_OPT  // Disable trellis in estimate_yrd_for_sb
} UENUM1BYTE(TRELLIS_OPT_TYPE);

enum {
  FULL_TXFM_RD,
  LOW_TXFM_RD,
} UENUM1BYTE(TXFM_RD_MODEL);

enum {
  USE_FULL_RD = 0,
  USE_FAST_RD,
  USE_LARGESTALL,
} UENUM1BYTE(TX_SIZE_SEARCH_METHOD);

typedef struct macroblock_plane {
  DECLARE_ALIGNED(32, int16_t, src_diff[MAX_SB_SQUARE]);
  tran_low_t *dqcoeff;
  tran_low_t *qcoeff;
  tran_low_t *coeff;
  uint16_t *eobs;
  uint8_t *txb_entropy_ctx;
  struct buf_2d src;

  // Quantizer setings
  // These are used/accessed only in the quantization process
  // RDO does not / must not depend on any of these values
  // All values below share the coefficient scale/shift used in TX
  const int16_t *quant_fp_QTX;
  const int16_t *round_fp_QTX;
  const int16_t *quant_QTX;
  const int16_t *quant_shift_QTX;
  const int16_t *zbin_QTX;
  const int16_t *round_QTX;
  const int16_t *dequant_QTX;
} MACROBLOCK_PLANE;

typedef struct {
  int txb_skip_cost[TXB_SKIP_CONTEXTS][2];
  int base_eob_cost[SIG_COEF_CONTEXTS_EOB][3];
  int base_cost[SIG_COEF_CONTEXTS][8];
  int eob_extra_cost[EOB_COEF_CONTEXTS][2];
  int dc_sign_cost[DC_SIGN_CONTEXTS][2];
  int lps_cost[LEVEL_CONTEXTS][COEFF_BASE_RANGE + 1 + COEFF_BASE_RANGE + 1];
} LV_MAP_COEFF_COST;

typedef struct {
  int eob_cost[2][11];
} LV_MAP_EOB_COST;

typedef struct {
  tran_low_t tcoeff[MAX_MB_PLANE][MAX_SB_SQUARE];
  uint16_t eobs[MAX_MB_PLANE][MAX_SB_SQUARE / (TX_SIZE_W_MIN * TX_SIZE_H_MIN)];
  // Transform block entropy contexts.
  // Bits 0~3: txb_skip_ctx; bits 4~5: dc_sign_ctx.
  uint8_t entropy_ctx[MAX_MB_PLANE]
                     [MAX_SB_SQUARE / (TX_SIZE_W_MIN * TX_SIZE_H_MIN)];
} CB_COEFF_BUFFER;

typedef struct {
  // TODO(angiebird): Reduce the buffer size according to sb_type
  CANDIDATE_MV ref_mv_stack[MODE_CTX_REF_FRAMES][USABLE_REF_MV_STACK_SIZE];
  uint16_t weight[MODE_CTX_REF_FRAMES][USABLE_REF_MV_STACK_SIZE];
  int_mv global_mvs[REF_FRAMES];
  int16_t mode_context[MODE_CTX_REF_FRAMES];
  uint8_t ref_mv_count[MODE_CTX_REF_FRAMES];
} MB_MODE_INFO_EXT;

// Structure to store best mode information at frame level. This
// frame level information will be used during bitstream preparation stage.
typedef struct {
  CANDIDATE_MV ref_mv_stack[USABLE_REF_MV_STACK_SIZE];
  uint16_t weight[USABLE_REF_MV_STACK_SIZE];
  // TODO(Ravi/Remya): Reduce the buffer size of global_mvs
  int_mv global_mvs[REF_FRAMES];
  int cb_offset;
  int16_t mode_context;
  uint8_t ref_mv_count;
} MB_MODE_INFO_EXT_FRAME;

typedef struct {
  uint8_t best_palette_color_map[MAX_PALETTE_SQUARE];
  int kmeans_data_buf[2 * MAX_PALETTE_SQUARE];
} PALETTE_BUFFER;

typedef struct {
  TX_SIZE tx_size;
  TX_SIZE inter_tx_size[INTER_TX_SIZE_BUF_LEN];
  uint8_t blk_skip[MAX_MIB_SIZE * MAX_MIB_SIZE];
  uint8_t tx_type_map[MAX_MIB_SIZE * MAX_MIB_SIZE];
  RD_STATS rd_stats;
  uint32_t hash_value;
} MB_RD_INFO;

#define RD_RECORD_BUFFER_LEN 8
typedef struct {
  MB_RD_INFO tx_rd_info[RD_RECORD_BUFFER_LEN];  // Circular buffer.
  int index_start;
  int num;
  CRC32C crc_calculator;  // Hash function.
} MB_RD_RECORD;

typedef struct {
  int64_t dist;
  int64_t sse;
  int rate;
  uint16_t eob;
  TX_TYPE tx_type;
  uint16_t entropy_context;
  uint8_t txb_entropy_ctx;
  uint8_t valid;
  uint8_t fast;  // This is not being used now.
  uint8_t perform_block_coeff_opt;
} TXB_RD_INFO;

#define TX_SIZE_RD_RECORD_BUFFER_LEN 256
typedef struct {
  uint32_t hash_vals[TX_SIZE_RD_RECORD_BUFFER_LEN];
  TXB_RD_INFO tx_rd_info[TX_SIZE_RD_RECORD_BUFFER_LEN];
  int index_start;
  int num;
} TXB_RD_RECORD;

typedef struct tx_size_rd_info_node {
  TXB_RD_INFO *rd_info_array;  // Points to array of size TX_TYPES.
  struct tx_size_rd_info_node *children[4];
} TXB_RD_INFO_NODE;

// Simple translation rd state for prune_comp_search_by_single_result
typedef struct {
  RD_STATS rd_stats;
  RD_STATS rd_stats_y;
  RD_STATS rd_stats_uv;
  uint8_t blk_skip[MAX_MIB_SIZE * MAX_MIB_SIZE];
  uint8_t tx_type_map[MAX_MIB_SIZE * MAX_MIB_SIZE];
  uint8_t skip_txfm;
  uint8_t disable_skip_txfm;
  uint8_t early_skipped;
} SimpleRDState;

// 4: NEAREST, NEW, NEAR, GLOBAL
#define SINGLE_REF_MODES ((REF_FRAMES - 1) * 4)

#define MAX_COMP_RD_STATS 64
typedef struct {
  int32_t rate[COMPOUND_TYPES];
  int64_t dist[COMPOUND_TYPES];
  int32_t model_rate[COMPOUND_TYPES];
  int64_t model_dist[COMPOUND_TYPES];
  int comp_rs2[COMPOUND_TYPES];
  int_mv mv[2];
  MV_REFERENCE_FRAME ref_frames[2];
  PREDICTION_MODE mode;
  int_interpfilters filter;
  int ref_mv_idx;
  int is_global[2];
  INTERINTER_COMPOUND_DATA interinter_comp;
} COMP_RD_STATS;

// Struct for buffers used by av1_compound_type_rd() function.
// For sizes and alignment of these arrays, refer to
// alloc_compound_type_rd_buffers() function.
typedef struct {
  uint8_t *pred0;
  uint8_t *pred1;
  int16_t *residual1;          // src - pred1
  int16_t *diff10;             // pred1 - pred0
  uint8_t *tmp_best_mask_buf;  // backup of the best segmentation mask
} CompoundTypeRdBuffers;

// Struct for buffers used to speed up rdopt for obmc.
// See the comments for calc_target_weighted_pred for details.
typedef struct {
  // A new source weighted with the above and left predictors for efficient
  // rdopt in obmc mode.
  int32_t *wsrc;
  // A new mask constructed from the original left and horizontal masks for
  // fast obmc rdopt.
  int32_t *mask;
  // Holds a prediction using the above/left predictor. This is used to build
  // the obmc predictor.
  uint8_t *above_pred;
  uint8_t *left_pred;
} OBMCBuffer;

// This struct holds some parameters related to partitioning schemes in av1.
// TODO(chiyotsai@google.com): Consolidate this with SIMPLE_MOTION_DATA_TREE
typedef struct {
#if !CONFIG_REALTIME_ONLY
  // The following 4 parameters are used for cnn-based partitioning on intra
  // frame.
  // Where we are on the quad tree. Used to index into the cnn buffer for
  // partition decision.
  int quad_tree_idx;
  // Whether the CNN buffer contains valid output
  int cnn_output_valid;
  // A buffer used by our segmentation CNN for intra-frame partitioning.
  float cnn_buffer[CNN_OUT_BUF_SIZE];
  // log of the quantization parameter of the current BLOCK_64X64 that includes
  // the current block. Used as an input to the CNN.
  float log_q;
#endif

  // Holds the variable of various subblocks. This is used by rt mode for
  // variance based partitioning.
  //   0    - 128x128  |  1-2   - 128x64  |   3-4   -  64x128
  //  5-8   -  64x64   |  9-16  -  64x32  |  17-24  -  32x64
  // 25-40  -  32x32
  // 41-104 -  16x16
  uint8_t variance_low[105];
} PartitionSearchInfo;

// This struct stores the parameters used to perform the txfm search. For the
// most part, this determines how various speed features are used.
typedef struct {
  // Whether to limit the txfm search type to the default txfm during rdopt.
  // This could either be a result of either sequence parameter or speed
  // features.
  int use_default_intra_tx_type;
  int use_default_inter_tx_type;

  // Try to prune 2d transforms based on 1d transform results.
  int prune_2d_txfm_mode;

  // The following six parameters are copied from WinnerModeParams based on the
  // current evaluation mode. See the documentation for WinnerModeParams for
  // more detail.
  unsigned int coeff_opt_dist_threshold;
  unsigned int coeff_opt_satd_threshold;
  unsigned int tx_domain_dist_threshold;
  TX_SIZE_SEARCH_METHOD tx_size_search_method;
  unsigned int use_transform_domain_distortion;
  unsigned int skip_txfm_level;

  // Although this looks suspicious similar to a bitstream element, This
  // tx_mode_search_type is used internally by the encoder, and is not
  // written to the bitstream. It determines what kind of tx_mode should be
  // searched. For example, we might set it to TX_MODE_LARGEST to find a good
  // candidate, then use TX_MODE_SELECT on it
  TX_MODE tx_mode_search_type;
} TxfmSearchParams;

#define MAX_NUM_8X8_TXBS ((MAX_MIB_SIZE >> 1) * (MAX_MIB_SIZE >> 1))
#define MAX_NUM_16X16_TXBS ((MAX_MIB_SIZE >> 2) * (MAX_MIB_SIZE >> 2))
#define MAX_NUM_32X32_TXBS ((MAX_MIB_SIZE >> 3) * (MAX_MIB_SIZE >> 3))
#define MAX_NUM_64X64_TXBS ((MAX_MIB_SIZE >> 4) * (MAX_MIB_SIZE >> 4))

// This struct stores various encoding/search decisions related to txfm search.
// This can include cache of previous txfm results, the current encoding
// decision, etc.
typedef struct {
  // Skips transform and quantization on a partition block level.
  int skip_txfm;

  // Skips transform and quantization on a transform block level inside the
  // current partition block. Each element of this array is used as a bit-field.
  // So for example, the we are skipping on the luma plane, then the last bit
  // would be set to 1.
  uint8_t blk_skip[MAX_MIB_SIZE * MAX_MIB_SIZE];

  // Keeps a record of what kind of transform to use for each of the transform
  // block inside the partition block. A quick note here: the buffer here is
  // NEVER directly used. Instead, this just allocates the memory for
  // MACROBLOCKD::tx_type_map during rdopt on the partition block. So if we need
  // to save memory, we could move the allocation to pick_sb_mode instead.
  uint8_t tx_type_map_[MAX_MIB_SIZE * MAX_MIB_SIZE];

  // Records of a partition block's inter-mode txfm result hashed by its
  // residue. This is similar to txb_rd_record_*, but this operates on the whole
  // prediction block.
  MB_RD_RECORD mb_rd_record;

  // Records of a transform block's result hashed by residue within the
  // transform block. This operates on txb level only and only applies to square
  // txfms.
  // Inter transform block RD search
  TXB_RD_RECORD txb_rd_record_8X8[MAX_NUM_8X8_TXBS];
  TXB_RD_RECORD txb_rd_record_16X16[MAX_NUM_16X16_TXBS];
  TXB_RD_RECORD txb_rd_record_32X32[MAX_NUM_32X32_TXBS];
  TXB_RD_RECORD txb_rd_record_64X64[MAX_NUM_64X64_TXBS];
  // Intra transform block RD search
  TXB_RD_RECORD txb_rd_record_intra;

  // Keep track of how many times we've used split partition for transform
  // blocks. Misleadingly, his parameter doesn't actually keep track of the
  // count of the current block. Instead, it's a cumulative count across of the
  // whole frame. The main usage is that if txb_split_count is zero, then we
  // can signal TX_MODE_LARGEST at frame level.
  // TODO(chiyotsai@google.com): Move this to a more appropriate location such
  // as ThreadData.
  unsigned int txb_split_count;
#if CONFIG_SPEED_STATS
  // For debugging. Used to check how many txfm searches we are doing.
  unsigned int tx_search_count;
#endif  // CONFIG_SPEED_STATS
} TxfmSearchInfo;

// This struct holds the entropy costs for various modes sent to the bitstream.
// This however does not include the costs for mv and transformed coefficients.
typedef struct {
  // ==========================================================================
  // Partition Costs
  // ==========================================================================
  int partition_cost[PARTITION_CONTEXTS][EXT_PARTITION_TYPES];

  // ==========================================================================
  // Intra Mode Costs
  // ==========================================================================
  int mbmode_cost[BLOCK_SIZE_GROUPS][INTRA_MODES];  // Mode cost for inter frame
  int y_mode_costs[INTRA_MODES][INTRA_MODES][INTRA_MODES];  // Mode cost for kf
  int intra_uv_mode_cost[CFL_ALLOWED_TYPES][INTRA_MODES][UV_INTRA_MODES];
  int filter_intra_cost[BLOCK_SIZES_ALL][2];
  int filter_intra_mode_cost[FILTER_INTRA_MODES];
  int angle_delta_cost[DIRECTIONAL_MODES][2 * MAX_ANGLE_DELTA + 1];

  // Screen Content Tools Costs
  int intrabc_cost[2];

  int palette_y_size_cost[PALATTE_BSIZE_CTXS][PALETTE_SIZES];
  int palette_uv_size_cost[PALATTE_BSIZE_CTXS][PALETTE_SIZES];
  int palette_y_color_cost[PALETTE_SIZES][PALETTE_COLOR_INDEX_CONTEXTS]
                          [PALETTE_COLORS];
  int palette_uv_color_cost[PALETTE_SIZES][PALETTE_COLOR_INDEX_CONTEXTS]
                           [PALETTE_COLORS];
  int palette_y_mode_cost[PALATTE_BSIZE_CTXS][PALETTE_Y_MODE_CONTEXTS][2];
  int palette_uv_mode_cost[PALETTE_UV_MODE_CONTEXTS][2];

  // The rate associated with each alpha codeword
  int cfl_cost[CFL_JOINT_SIGNS][CFL_PRED_PLANES][CFL_ALPHABET_SIZE];

  // ==========================================================================
  // Inter Mode Costs
  // ==========================================================================
  int skip_mode_cost[SKIP_MODE_CONTEXTS][2];

  // MV Mode Costs
  int newmv_mode_cost[NEWMV_MODE_CONTEXTS][2];
  int zeromv_mode_cost[GLOBALMV_MODE_CONTEXTS][2];
  int refmv_mode_cost[REFMV_MODE_CONTEXTS][2];
  int drl_mode_cost0[DRL_MODE_CONTEXTS][2];

  // Ref Costs
  int single_ref_cost[REF_CONTEXTS][SINGLE_REFS - 1][2];
  int comp_inter_cost[COMP_INTER_CONTEXTS][2];
  int comp_ref_type_cost[COMP_REF_TYPE_CONTEXTS]
                        [CDF_SIZE(COMP_REFERENCE_TYPES)];
  int uni_comp_ref_cost[UNI_COMP_REF_CONTEXTS][UNIDIR_COMP_REFS - 1]
                       [CDF_SIZE(2)];
  // Cost for signaling ref_frame[0] (LAST_FRAME, LAST2_FRAME, LAST3_FRAME or
  // GOLDEN_FRAME) in bidir-comp mode.
  int comp_ref_cost[REF_CONTEXTS][FWD_REFS - 1][2];
  // Cost for signaling ref_frame[1] (ALTREF_FRAME, ALTREF2_FRAME, or
  // BWDREF_FRAME) in bidir-comp mode.
  int comp_bwdref_cost[REF_CONTEXTS][BWD_REFS - 1][2];

  // Compound Costs
  int intra_inter_cost[INTRA_INTER_CONTEXTS][2];
  int inter_compound_mode_cost[INTER_MODE_CONTEXTS][INTER_COMPOUND_MODES];
  int compound_type_cost[BLOCK_SIZES_ALL][MASKED_COMPOUND_TYPES];
  int wedge_idx_cost[BLOCK_SIZES_ALL][16];
  int interintra_cost[BLOCK_SIZE_GROUPS][2];
  int wedge_interintra_cost[BLOCK_SIZES_ALL][2];
  int interintra_mode_cost[BLOCK_SIZE_GROUPS][INTERINTRA_MODES];

  // Masks
  int comp_idx_cost[COMP_INDEX_CONTEXTS][2];
  int comp_group_idx_cost[COMP_GROUP_IDX_CONTEXTS][2];

  // Motion Mode/Filter Costs
  int motion_mode_cost[BLOCK_SIZES_ALL][MOTION_MODES];
  int motion_mode_cost1[BLOCK_SIZES_ALL][2];
  int switchable_interp_costs[SWITCHABLE_FILTER_CONTEXTS][SWITCHABLE_FILTERS];

  // ==========================================================================
  // Txfm Mode Costs
  // ==========================================================================
  int skip_txfm_cost[SKIP_CONTEXTS][2];
  int tx_size_cost[TX_SIZES - 1][TX_SIZE_CONTEXTS][TX_SIZES];
  int txfm_partition_cost[TXFM_PARTITION_CONTEXTS][2];
  int inter_tx_type_costs[EXT_TX_SETS_INTER][EXT_TX_SIZES][TX_TYPES];
  int intra_tx_type_costs[EXT_TX_SETS_INTRA][EXT_TX_SIZES][INTRA_MODES]
                         [TX_TYPES];

  // ==========================================================================
  // Restoration Mode Costs
  // ==========================================================================
  int switchable_restore_cost[RESTORE_SWITCHABLE_TYPES];
  int wiener_restore_cost[2];
  int sgrproj_restore_cost[2];
} ModeCosts;

// This struct holds the rates needed to transmit a new mv and the cost
// multiplier that converts entropy cost to sad/sse/var during motion search.
typedef struct {
  // A multiplier that converts mv cost to l2 error.
  int errorperbit;
  // A multiplier that converts mv cost to l1 error.
  int sadperbit;

  int nmv_joint_cost[MV_JOINTS];

  // Below are the entropy costs needed to encode a given mv.
  // nmv_costs_(hp_)alloc are two arrays that holds the memory
  // for holding the mv cost. But since the motion vectors can be negative, we
  // shift them to the middle and store the resulting pointer in nmvcost(_hp)
  // for easier referencing. Finally, nmv_cost_stack points to the nmvcost array
  // with the mv precision we are currently working with. In essence, only
  // mv_cost_stack is needed for motion search, the other can be considered
  // private.
  int nmv_cost_alloc[2][MV_VALS];
  int nmv_cost_hp_alloc[2][MV_VALS];
  int *nmv_cost[2];
  int *nmv_cost_hp[2];
  int **mv_cost_stack;
} MvCosts;

// This struct holds the costs need to encode the coefficients
typedef struct {
  LV_MAP_COEFF_COST coeff_costs[TX_SIZES][PLANE_TYPES];
  LV_MAP_EOB_COST eob_costs[7][2];
} CoeffCosts;

struct inter_modes_info;
typedef struct macroblock MACROBLOCK;
struct macroblock {
  // ==========================================================================
  // Source, Buffers and Decoder
  // ==========================================================================
  // Holds the src buffer for each of plane of the current block. This
  // also contains the txfm and quantized txfm coefficients.
  struct macroblock_plane plane[MAX_MB_PLANE];

  // Contains the encoder's copy of what the decoder sees in the current block.
  // Most importantly, this struct contains pointers to mbmi that is used in
  // final bitstream packing.
  MACROBLOCKD e_mbd;

  // Contains extra information not transmitted in the bitstream but are
  // derived. For example, this contains the stack of ref_mvs.
  MB_MODE_INFO_EXT *mbmi_ext;

  // Contains the finalized info in mbmi_ext that gets used at the frame level
  // for bitstream packing.
  MB_MODE_INFO_EXT_FRAME *mbmi_ext_frame;

  FRAME_CONTEXT *row_ctx;
  // This context will be used to update color_map_cdf pointer which would be
  // used during pack bitstream. For single thread and tile-multithreading case
  // this pointer will be same as xd->tile_ctx, but for the case of row-mt:
  // xd->tile_ctx will point to a temporary context while tile_pb_ctx will point
  // to the accurate tile context.
  FRAME_CONTEXT *tile_pb_ctx;

  // Points to cb_coef_buff in the AV1_COMP struct, which contains the finalized
  // coefficients. This is here to conveniently copy the best coefficients to
  // frame level for bitstream packing. Since CB_COEFF_BUFFER is allocated on a
  // superblock level, we need to combine it with cb_offset to get the proper
  // position for the current coding block.
  CB_COEFF_BUFFER *cb_coef_buff;
  uint16_t cb_offset;

  // Stores some modified source and masks used for fast OBMC search.
  OBMCBuffer obmc_buffer;
  // Stores the best palette map.
  PALETTE_BUFFER *palette_buffer;
  // Stores buffers used to perform compound_type_rd.
  CompoundTypeRdBuffers comp_rd_buffer;
  // Stores convolution during the averaging prediction in compound/ mode.
  CONV_BUF_TYPE *tmp_conv_dst;

  // Points to a buffer that is used to hold temporary prediction results. This
  // is used in two ways:
  // 1. This is a temporary buffer used to pingpong the prediction in
  //    handle_inter_mode.
  // 2. xd->tmp_obmc_bufs also points to this buffer, and is used in ombc
  //    prediction.
  uint8_t *tmp_pred_bufs[2];

  // ==========================================================================
  // Costs for Rdopt
  // ==========================================================================
  // The quantization index for the current partition block. This is used to
  // as the index to find quantization parameter for luma and chroma transformed
  // coefficients.
  int qindex;

  // The difference between the frame-level base qindex and the qindex used for
  // the current superblock. This is used to track whether a non-zero delta for
  // qindex is used at least once in the current frame.
  int delta_qindex;

  // The rd multiplier used to determine the rate-distortion trade-off. This is
  // roughly proportional to the inverse of q-index for a given frame, but this
  // can be manipulated to for better rate-control. For example, in tune_ssim
  // mode, this is scaled by a factor related to the variance of the current
  // block.
  int rdmult;

  // These are measure of the energy in the current source mb/sb. They are used
  // to determine the rdmult to facilitate better rdopt.
  int mb_energy;
  int sb_energy_level;

  // Stores the rate needed to signal a mode to the bitstream.
  ModeCosts mode_costs;

  // Stores the rate needed to encode a new motion vector to the bitstream
  // and some multipliers for motion search.
  MvCosts mv_costs;

  // Stores the rate needed to signal the txfm coefficients to the bitstream.
  CoeffCosts coeff_costs;

  // ==========================================================================
  // Segmentation
  // ==========================================================================
  // Part of the segmentation mode. In skip_block mode, all mvs are set 0 and
  // all txfms are skipped.
  int seg_skip_block;

  // ==========================================================================
  // Superblock
  // ==========================================================================
  // Stores information on a whole superblock level.
  // TODO(chiyotsai@google.com): Refactor this out of macroblock
  SuperBlockEnc sb_enc;

  // The characteristic of the current superblock. e.g., it can have high sad,
  // low sad, etc. Only used by realtime mode.
  uint8_t content_state_sb;

  // ==========================================================================
  // Reference Frame Search
  // ==========================================================================
  // The sad of the predicted mv for each of the reference frame. This is used
  // to measure how viable a reference frames.
  int pred_mv_sad[REF_FRAMES];
  // Contains min(pred_mv_sad).
  int best_pred_mv_sad;

  // Determines whether a given ref frame is "good" based on data from the TPL
  // model. If so, this stops selective_ref frame from pruning the given ref
  // frame at block level.
  uint8_t tpl_keep_ref_frame[REF_FRAMES];

  // Keeps track of ref frames that are selected by square partition blocks
  // within a superblock, in MI resolution. They can be used to prune ref frames
  // for rectangular blocks.
  int picked_ref_frames_mask[MAX_MIB_SIZE * MAX_MIB_SIZE];

  // Determines whether to prune reference frames in real-time mode. For the
  // most part, this is the same as nonrd_prune_ref_frame_search in
  // cpi->sf.rt_sf.nonrd_prune_ref_frame_search, but this can be selectively
  // turned off if the only frame available is GOLDEN_FRAME.
  int nonrd_prune_ref_frame_search;

  // ==========================================================================
  // Partition Search
  // ==========================================================================
  // Stores some partition-search related buffers.
  PartitionSearchInfo part_search_info;

  // In some cases, our speed features can be overly aggressive and remove all
  // modes search in the superblock. In this case, we set
  // must_find_valid_partition to 1 to reduce the number of speed features, and
  // recode the superblock again.
  int must_find_valid_partition;

  // ==========================================================================
  // Prediction Mode Search
  // ==========================================================================
  // Skip mode tries to use the closest forward and backward references for
  // inter prediction. Skip here means to skip transmitting the reference
  // frames, not to be confused with skip_txfm.
  int skip_mode;

  // Determines a rd threshold to determine whether to continue searching the
  // current mode. If the current best rd is already <= threshold, then we skip
  // the current mode.
  int thresh_freq_fact[BLOCK_SIZES_ALL][MAX_MODES];

  // Winner mode is a two-pass strategy to find the best prediction mode. In the
  // first pass, we search the prediction modes with a limited set of txfm
  // options, and keep the top modes. These modes are called the winner modes.
  // In the second pass, we retry the winner modes with more thorough txfm
  // options.
  // Stores the winner modes.
  WinnerModeStats winner_mode_stats[AOMMAX(MAX_WINNER_MODE_COUNT_INTRA,
                                           MAX_WINNER_MODE_COUNT_INTER)];
  // Tracks how many winner modes there are.
  int winner_mode_count;

  // These are for inter_mode_rd_model_estimation, which is another two pass
  // approach. In this speed feature, we collect data in the first couple frames
  // to build an rd model to estimate the rdcost of a prediction model based on
  // the residue error. Once enough data is collected, this speed feature uses
  // the estimated rdcost to find the most performant prediction mode. Then we
  // follow up with a second pass find the best transform for the mode.
  // Determines if one would go with reduced complexity transform block
  // search model to select prediction modes, or full complexity model
  // to select transform kernel.
  TXFM_RD_MODEL rd_model;

  // Stores the inter mode information needed to build an rd model.
  // TODO(any): try to consolidate this speed feature with winner mode
  // processing.
  struct inter_modes_info *inter_modes_info;

  // Yet another 2-pass approach that tries to prune compound mode by first
  // doing a simple_translational search on single ref modes. This however does
  // not have good trade-off so it is only used by real-time mode.
  SimpleRDState simple_rd_state[SINGLE_REF_MODES][3];

  // Determines how to blend the compound predictions
  uint8_t compound_idx;

  // Caches the results of compound type search so they can be reused later.
  COMP_RD_STATS comp_rd_stats[MAX_COMP_RD_STATS];
  int comp_rd_stats_idx;

  // The edge strengths are used in wedge_search.
  // The likelihood of an edge existing in the block (using partial Canny edge
  // detection). For reference, 556 is the value returned for a solid
  // vertical black/white edge.
  uint16_t edge_strength;
  // The strongest edge strength seen along the x/y axis.
  uint16_t edge_strength_x;
  uint16_t edge_strength_y;

  // Contains the hash table, hash function, and buffer used for intrabc.
  IntraBCHashInfo intrabc_hash_info;

  // ==========================================================================
  // MV Search
  // ==========================================================================
  // The l_\inf norm of the best ref_mv for each frame. This is used to
  // determine the initial step size during motion search.
  unsigned int max_mv_context[REF_FRAMES];

  // These define limits to motion vector components to prevent them
  // from extending outside the UMV borders
  FullMvLimits mv_limits;

  // In interpolation search, we can usually skip recalculating the luma
  // prediction because it is already calculated by a previous predictor. This
  // flag signifies that some modes might have been skipped, so we need to redo
  // the motion compensation.
  int recalc_luma_mc_data;

  // ==========================================================================
  // Txfm Search
  // ==========================================================================
  // Stores various txfm search related parameters such as txfm_type, txfm_size,
  // trellis eob search, etc.
  TxfmSearchParams txfm_search_params;

  // Caches old txfm search results and keeps the current txfm decisions.
  TxfmSearchInfo txfm_search_info;

  // Strong color activity detection. Used in REALTIME coding mode to enhance
  // the visual quality at the boundary of moving color objects.
  uint8_t color_sensitivity[2];

  // ==========================================================================
  // Misc
  // ==========================================================================
  // Variance on the source frame.
  unsigned int source_variance;
  // The sse of the current predictor.
  unsigned int pred_sse[REF_FRAMES];

  // ==========================================================================
  // Unused
  // ==========================================================================
  // Some of these are not currently used by the codec so they should probably
  // be removed.
  unsigned int simple_motion_pred_sse;
};

// Only consider full SB, MC_FLOW_BSIZE_1D = 16.
static INLINE int tpl_blocks_in_sb(BLOCK_SIZE bsize) {
  switch (bsize) {
    case BLOCK_64X64: return 16;
    case BLOCK_128X128: return 64;
    default: assert(0);
  }
  return -1;
}

static INLINE int is_rect_tx_allowed_bsize(BLOCK_SIZE bsize) {
  static const char LUT[BLOCK_SIZES_ALL] = {
    0,  // BLOCK_4X4
    1,  // BLOCK_4X8
    1,  // BLOCK_8X4
    0,  // BLOCK_8X8
    1,  // BLOCK_8X16
    1,  // BLOCK_16X8
    0,  // BLOCK_16X16
    1,  // BLOCK_16X32
    1,  // BLOCK_32X16
    0,  // BLOCK_32X32
    1,  // BLOCK_32X64
    1,  // BLOCK_64X32
    0,  // BLOCK_64X64
    0,  // BLOCK_64X128
    0,  // BLOCK_128X64
    0,  // BLOCK_128X128
    1,  // BLOCK_4X16
    1,  // BLOCK_16X4
    1,  // BLOCK_8X32
    1,  // BLOCK_32X8
    1,  // BLOCK_16X64
    1,  // BLOCK_64X16
  };

  return LUT[bsize];
}

static INLINE int is_rect_tx_allowed(const MACROBLOCKD *xd,
                                     const MB_MODE_INFO *mbmi) {
  return is_rect_tx_allowed_bsize(mbmi->sb_type) &&
         !xd->lossless[mbmi->segment_id];
}

static INLINE int tx_size_to_depth(TX_SIZE tx_size, BLOCK_SIZE bsize) {
  TX_SIZE ctx_size = max_txsize_rect_lookup[bsize];
  int depth = 0;
  while (tx_size != ctx_size) {
    depth++;
    ctx_size = sub_tx_size_map[ctx_size];
    assert(depth <= MAX_TX_DEPTH);
  }
  return depth;
}

static INLINE void set_blk_skip(uint8_t txb_skip[], int plane, int blk_idx,
                                int skip) {
  if (skip)
    txb_skip[blk_idx] |= 1UL << plane;
  else
    txb_skip[blk_idx] &= ~(1UL << plane);
#ifndef NDEBUG
  // Set chroma planes to uninitialized states when luma is set to check if
  // it will be set later
  if (plane == 0) {
    txb_skip[blk_idx] |= 1UL << (1 + 4);
    txb_skip[blk_idx] |= 1UL << (2 + 4);
  }

  // Clear the initialization checking bit
  txb_skip[blk_idx] &= ~(1UL << (plane + 4));
#endif
}

static INLINE int is_blk_skip(uint8_t *txb_skip, int plane, int blk_idx) {
#ifndef NDEBUG
  // Check if this is initialized
  assert(!(txb_skip[blk_idx] & (1UL << (plane + 4))));

  // The magic number is 0x77, this is to test if there is garbage data
  assert((txb_skip[blk_idx] & 0x88) == 0);
#endif
  return (txb_skip[blk_idx] >> plane) & 1;
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // AOM_AV1_ENCODER_BLOCK_H_
