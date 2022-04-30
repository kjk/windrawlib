#ifndef WD_BACKEND_D2D_H
#define WD_BACKEND_D2D_H

#include "misc.h"
#include "dummy/d2d1.h"


void d2d_reset_clip(d2d_canvas_t* c);

void d2d_reset_transform(d2d_canvas_t* c);
void d2d_apply_transform(d2d_canvas_t* c, const dummy_D2D1_MATRIX_3X2_F* matrix);

/* Note: Can be called only if D2D_CANVASFLAG_RTL, and have to reinstall
 * the original transformation then-after. */
void d2d_disable_rtl_transform(d2d_canvas_t* c, dummy_D2D1_MATRIX_3X2_F* old_matrix);

void d2d_setup_arc_segment(dummy_D2D1_ARC_SEGMENT* arc_seg,
                           float cx, float cy, float rx, float ry,
                    float base_angle, float sweep_angle);
void d2d_setup_bezier_segment(dummy_D2D1_BEZIER_SEGMENT* bezier_seg,
                              float x0, float y0, float x1, float y1,
                              float x2, float y2);
dummy_ID2D1Geometry* d2d_create_arc_geometry(float cx, float cy, float rx, float ry,
                    float base_angle, float sweep_angle, BOOL pie);


#endif  /* WD_BACKEND_D2D_H */
