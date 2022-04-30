#include "backend-d2d.h"
#include "lock.h"

void
d2d_disable_rtl_transform(d2d_canvas_t* c, dummy_D2D1_MATRIX_3X2_F* old_matrix)
{
    dummy_D2D1_MATRIX_3X2_F r;    /* Reflection + transition for WD_CANVAS_LAYOUTRTL. */
    dummy_D2D1_MATRIX_3X2_F ur;   /* R * user's transformation. */
    dummy_D2D1_MATRIX_3X2_F u;    /* Only user's transformation. */

    r._11 = -1.0f;				r._12 = 0.0f;
    r._21 = 0.0f;				r._22 = 1.0f;
    r._31 = (float) c->width;	r._32 = 0.0f;

    dummy_ID2D1RenderTarget_GetTransform(c->target, &ur);
    if(old_matrix != NULL)
        memcpy(old_matrix, &ur, sizeof(dummy_D2D1_MATRIX_3X2_F));
    ur._31 += D2D_BASEDELTA_X;
    ur._32 -= D2D_BASEDELTA_Y;

    /* Note R is inverse to itself. */
    d2d_matrix_mult(&u, &ur, &r);

    dummy_ID2D1RenderTarget_SetTransform(c->target, &u);
}

void
d2d_setup_arc_segment(dummy_D2D1_ARC_SEGMENT* arc_seg, float cx, float cy, float rx, float ry,
                      float base_angle, float sweep_angle)
{
    float sweep_rads = (base_angle + sweep_angle) * (WD_PI / 180.0f);

    arc_seg->point.x = cx + rx * cosf(sweep_rads);
    arc_seg->point.y = cy + ry * sinf(sweep_rads);
    arc_seg->size.width = rx;
    arc_seg->size.height = ry;
    arc_seg->rotationAngle = 0.0f;

    if(sweep_angle >= 0.0f)
        arc_seg->sweepDirection = dummy_D2D1_SWEEP_DIRECTION_CLOCKWISE;
    else
        arc_seg->sweepDirection = dummy_D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;

    if(sweep_angle >= 180.0f)
        arc_seg->arcSize = dummy_D2D1_ARC_SIZE_LARGE;
    else
        arc_seg->arcSize = dummy_D2D1_ARC_SIZE_SMALL;
}

void
d2d_setup_bezier_segment(dummy_D2D1_BEZIER_SEGMENT* bezier_seg, float x0, float y0, float x1, float y1,
                      float x2, float y2)
{
    bezier_seg->point1.x = x0;
    bezier_seg->point1.y = y0;
    bezier_seg->point2.x = x1;
    bezier_seg->point2.y = y1;
    bezier_seg->point3.x = x2;
    bezier_seg->point3.y = y2;
}

dummy_ID2D1Geometry*
d2d_create_arc_geometry(float cx, float cy, float rx, float ry,
                        float base_angle, float sweep_angle, BOOL pie)
{
    dummy_ID2D1PathGeometry* g = NULL;
    dummy_ID2D1GeometrySink* s;
    HRESULT hr;
    float base_rads = base_angle * (WD_PI / 180.0f);
    dummy_D2D1_POINT_2F pt;
    dummy_D2D1_ARC_SEGMENT arc_seg;

    wd_lock();
    hr = dummy_ID2D1Factory_CreatePathGeometry(d2d_factory, &g);
    wd_unlock();
    if(FAILED(hr)) {
        WD_TRACE_HR("d2d_create_arc_geometry: "
                    "ID2D1Factory::CreatePathGeometry() failed.");
        return NULL;
    }
    hr = dummy_ID2D1PathGeometry_Open(g, &s);
    if(FAILED(hr)) {
        WD_TRACE_HR("d2d_create_arc_geometry: ID2D1PathGeometry::Open() failed.");
        dummy_ID2D1PathGeometry_Release(g);
        return NULL;
    }

    pt.x = cx + rx * cosf(base_rads);
    pt.y = cy + ry * sinf(base_rads);
    dummy_ID2D1GeometrySink_BeginFigure(s, pt, dummy_D2D1_FIGURE_BEGIN_FILLED);

    d2d_setup_arc_segment(&arc_seg, cx, cy, rx, ry, base_angle, sweep_angle);
    dummy_ID2D1GeometrySink_AddArc(s, &arc_seg);

    if(pie) {
        pt.x = cx;
        pt.y = cy;
        dummy_ID2D1GeometrySink_AddLine(s, pt);
        dummy_ID2D1GeometrySink_EndFigure(s, dummy_D2D1_FIGURE_END_CLOSED);
    } else {
        dummy_ID2D1GeometrySink_EndFigure(s, dummy_D2D1_FIGURE_END_OPEN);
    }

    dummy_ID2D1GeometrySink_Close(s);
    dummy_ID2D1GeometrySink_Release(s);

    return (dummy_ID2D1Geometry*) g;
}
