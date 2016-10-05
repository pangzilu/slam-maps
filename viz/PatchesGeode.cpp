#include "PatchesGeode.hpp"

#include <vizkit3d/ColorConversionHelper.hpp>

namespace vizkit3d
{
    PatchesGeode::PatchesGeode(float x_res, float y_res)
        : vertex_index(0),
          xp(0), yp(0),
          xs(x_res), ys(y_res),
          hue(0.0),
          sat(1.0),
          alpha(1.0),
          lum(1.0),
          cycle_color(false),
          uncertaintyScale(1.0)
    {
        colors = new osg::Vec4Array;
        vertices = new osg::Vec3Array;
        normals = new osg::Vec3Array;
        geom = new osg::Geometry;
        var_vertices = new osg::Vec3Array;

        showNormals = true;
        showPatchExtents = false;

        geom->setUseVertexBufferObjects(true);

        geom->setVertexArray(vertices);
        geom->setNormalArray(normals);
        geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        geom->setColorArray(colors);
        geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

        addDrawable(geom);
    }
    void PatchesGeode::drawPlane(
            const float & zp,
            const float & height,
            const osg::Vec3& mean,
            const osg::Vec3& normal,
            const float & stdev)
    {
        const osg::Vec3 position(xp, yp, zp);
        const osg::Vec3 extents(xs*0.5f, ys*0.5f, height*0.5f);
#ifndef NDEBUG
        // Sanity check (disabled in release mode)
        for(int i=0; i<3; ++i)
            if( std::abs(mean[i]) > extents[i])
            {
                std::cerr << "Mean of SurfacePatch is outside of its extents! {";
                for(int j=0; j<3; ++j) std::cerr << mean[j] << (j==2? "}   {" : ", ");
                for(int j=0; j<3; ++j) std::cerr << extents[j] << (j==2? "}\n" : ", ");
                break;
            }
#endif
        // first calculate the intersections of the plane with the borders of the box
        // Here, `extents` and `mean` are relative to the origin of the box
        float dist = mean*normal; // scalar product gives the signed distance from the origin

        // find the max coefficient of the normal:
        int i=0, j=1, k=2;
        if(std::abs(normal[i]) < std::abs(normal[j])) std::swap(i,j);
        if(std::abs(normal[i]) < std::abs(normal[k])) std::swap(i,k);

        float dotj = extents[j]*normal[j];
        float dotk = extents[k]*normal[k];

        osg::Vec3 prev_p;
        enum { NONE, LOW, BOX, HIGH } prev_pos = NONE, pos;
        // calculate intersections in direction k:
        for(int n=0; n<5; ++n)
        {
            osg::Vec3 p(0,0,0);
            float dotp = 0.0f;
            if((n+1)&2)
                dotp += dotj, p[j] = extents[j];
            else
                dotp -= dotj, p[j] = -extents[j];
            if(n&2)
                dotp += dotk, p[k] = extents[k];
            else
                dotp -= dotk, p[k] = -extents[k];

            p[i] = (dist - dotp) / normal[i];

            if( p[i] < -extents[i])
                pos = LOW;
            else if( p[i] > extents[i])
                pos = HIGH;
            else
                pos = BOX;

            if( (prev_pos == LOW || prev_pos == HIGH) && pos != prev_pos )
            {
                // clipping in
                float h = prev_pos == LOW ? -extents[i] : extents[i];
                float s = (h - prev_p[i]) / (p[i] - prev_p[i]);
                osg::Vec3 cp = prev_p + (p - prev_p) * s;
                addVertex( position + cp, normal, stdev );
            }
            if( pos == BOX && n!=4 )
            {
                addVertex( position + p, normal, stdev );
            }
            else if( pos != prev_pos && prev_pos != NONE )
            {
                // clipping out
                float h = pos == LOW ? -extents[i] : extents[i];
                float s = (h - prev_p[i]) / (p[i] - prev_p[i]);
                osg::Vec3 cp = prev_p + (p - prev_p) * s;
                addVertex( position + cp, normal, stdev );
            }

            prev_pos = pos;
            prev_p = p;
        }

        closePolygon();

        osg::Vec3 center = position + mean;

        if(showNormals)
        {
            var_vertices->push_back(center);
            var_vertices->push_back(center+normal*0.1);
        }
        else if(showPatchExtents)
        {
            var_vertices->push_back(osg::Vec3(position.x(), position.y(), position.z() - extents.z()));
            var_vertices->push_back(osg::Vec3(position.x(), position.y(), position.z() + extents.z()));
        }
        else if(showUncertainty)
        {
            var_vertices->push_back(center-normal*stdev);
            var_vertices->push_back(center+normal*stdev);
        }

    }

    void PatchesGeode::drawPlane(
            const osg::Vec3& position,
            const osg::Vec4& heights,
            const osg::Vec3& extents,
            const osg::Vec3& normal,
            double min,
            double max)
    {
        const double xp = position.x();
        const double yp = position.y();
        //const double zp = position.z();

        const double xs = extents.x();
        const double ys = extents.y();

        enum { NONE, LOW, BOX, HIGH } prev_pos = NONE, pos;
        osg::Vec3 prev_p, p;
        for( size_t i=0; i<4; i++ )
        {
            switch( i%4 )
            {
            case 0: p = osg::Vec3(xp-xs*0.5, yp-ys*0.5, heights[0]); break;
            case 1: p = osg::Vec3(xp+xs*0.5, yp-ys*0.5, heights[1]); break;
            case 2: p = osg::Vec3(xp+xs*0.5, yp+ys*0.5, heights[2]); break;
            case 3: p = osg::Vec3(xp-xs*0.5, yp+ys*0.5, heights[3]); break;
            }

            if( p.z() < min )
                pos = LOW;
            else if( p.z() > max )
                pos = HIGH;
            else
                pos = BOX;

            if( (prev_pos == LOW || prev_pos == HIGH) && pos != prev_pos )
            {
                // clipping in
                double h = prev_pos == LOW ? min : max;
                double s = (h - prev_p.z()) / (p.z() - prev_p.z());
                osg::Vec3 cp = prev_p + (p - prev_p) * s;
                addVertex( cp, normal );
            }
            if( pos == BOX )
            {
                addVertex( p, normal );
            }
            else if( pos != prev_pos && prev_pos != NONE )
            {
                // clipping out
                double h = pos == LOW ? min : max;
                double s = (h - prev_p.z()) / (p.z() - prev_p.z());
                osg::Vec3 cp = prev_p + (p - prev_p) * s;
                addVertex( cp, normal );
            }

            prev_pos = pos;
            prev_p = p;
        }

        closePolygon();
    }

    void PatchesGeode::drawBox(
            const float& top,
            const float& height,
            const osg::Vec3& c_normal,
            const float & stdev )
    {
//        const double xp = position.x();
//        const double yp = position.y();
        const float zp = top - height*0.5f;

//        const double xs = extents.x();
//        const double ys = extents.y();
        const float zs = height;

        const osg::Vec4 h( osg::Vec4(zp,zp,zp,zp) );
        osg::Vec3 normal( c_normal );

        addVertex(osg::Vec3(xp-xs*0.5, yp-ys*0.5, h[0]+zs*0.5), normal, stdev);
        addVertex(osg::Vec3(xp+xs*0.5, yp-ys*0.5, h[1]+zs*0.5), normal, stdev);
        addVertex(osg::Vec3(xp+xs*0.5, yp+ys*0.5, h[2]+zs*0.5), normal, stdev);
        addVertex(osg::Vec3(xp-xs*0.5, yp+ys*0.5, h[3]+zs*0.5), normal, stdev);

        if( zs > 0.0 )
        {
            normal = osg::Vec3(0,-1.0,0);
            addVertex(osg::Vec3(xp-xs*0.5, yp-ys*0.5, h[0]+zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp+xs*0.5, yp-ys*0.5, h[1]+zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp+xs*0.5, yp-ys*0.5, h[2]-zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp-xs*0.5, yp-ys*0.5, h[3]-zs*0.5), normal, stdev);

            normal = osg::Vec3(1.0,0,0);
            addVertex(osg::Vec3(xp+xs*0.5, yp-ys*0.5, h[0]+zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp+xs*0.5, yp+ys*0.5, h[1]+zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp+xs*0.5, yp+ys*0.5, h[2]-zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp+xs*0.5, yp-ys*0.5, h[3]-zs*0.5), normal, stdev);

            normal = osg::Vec3(0,1.0,0);
            addVertex(osg::Vec3(xp+xs*0.5, yp+ys*0.5, h[0]+zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp-xs*0.5, yp+ys*0.5, h[1]+zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp-xs*0.5, yp+ys*0.5, h[2]-zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp+xs*0.5, yp+ys*0.5, h[3]-zs*0.5), normal, stdev);

            normal = osg::Vec3(-1.0,0,0);
            addVertex(osg::Vec3(xp-xs*0.5, yp+ys*0.5, h[0]+zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp-xs*0.5, yp-ys*0.5, h[1]+zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp-xs*0.5, yp-ys*0.5, h[2]-zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp-xs*0.5, yp+ys*0.5, h[3]-zs*0.5), normal, stdev);

            normal = osg::Vec3(0,0,-1.0);
            addVertex(osg::Vec3(xp-xs*0.5, yp-ys*0.5, h[0]-zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp+xs*0.5, yp-ys*0.5, h[1]-zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp+xs*0.5, yp+ys*0.5, h[2]-zs*0.5), normal, stdev);
            addVertex(osg::Vec3(xp-xs*0.5, yp+ys*0.5, h[3]-zs*0.5), normal, stdev);
        }

        closeQuads();

        const osg::Vec3 min(xp,yp,top-height);
        const osg::Vec3 max(xp,yp,top);

        if(showNormals)
        {
            var_vertices->push_back(max);
            var_vertices->push_back(max+normal*0.1);
        }
        else if(showPatchExtents)
        {
            var_vertices->push_back(osg::Vec3(min.x(), min.y(), min.z()));
            var_vertices->push_back(osg::Vec3(max.x(), max.y(), max.z()));
        }
        else if(showUncertainty)
        {
            var_vertices->push_back(min-normal*stdev);
            var_vertices->push_back(max+normal*stdev);
        }
    }

    void PatchesGeode::addVertex(const osg::Vec3& p, const osg::Vec3& n, const float & stdev)
    {
        vertices->push_back( p );
        normals->push_back( n );

        if( cycle_color )
        {
            hue = (p.z() - std::floor(p.z() / cycle_color_interval) * cycle_color_interval) / cycle_color_interval;
            alpha = std::max( 0.0, (uncertaintyScale - stdev) / uncertaintyScale);
            updateColor();
        }

        colors->push_back( color );
    }

    void PatchesGeode::updateColor()
    {
        vizkit3d::hslToRgb(hue, sat, lum , color.x(), color.y(), color.z());
        color.w() = alpha;
    }	

    void PatchesGeode::closePolygon()
    {
        geom->addPrimitiveSet(
                new osg::DrawArrays(
                        osg::PrimitiveSet::POLYGON,
                        vertex_index,
                        vertices->size() - vertex_index
                )
        );

        vertex_index = vertices->size();
    }

    void PatchesGeode::closeQuads()
    {
        geom->addPrimitiveSet(
                new osg::DrawArrays(
                        osg::PrimitiveSet::QUADS,
                        vertex_index,
                        vertices->size() - vertex_index
                )
        );

        vertex_index = vertices->size();
    }    

    void PatchesGeode::setColor(const osg::Vec4& color)
    {
        // TODO ideally this should also change the HSVA values
        this->color = color;
    }

    void PatchesGeode::setColorHSVA(float hue, float sat, float lum, float alpha)
    {
        this->hue = hue;
        this->sat = sat;
        this->alpha = alpha;
        this->lum = lum;

        updateColor();
    }

    void PatchesGeode::setCycleColorInterval(float cycle_color_interval)
    {
        this->cycle_color_interval = cycle_color_interval;
    }

    void PatchesGeode::showCycleColor(bool cycle_color)
    {
        this->cycle_color = cycle_color;
    }

    void PatchesGeode::setUncertaintyScale(double uncertainty_scale)
    {
        this->uncertaintyScale = uncertainty_scale;
    }

    void PatchesGeode::drawLines()
    {
        osg::ref_ptr<osg::Geometry> var_geom = new osg::Geometry;
        var_geom->setVertexArray( var_vertices );
        osg::ref_ptr<osg::DrawArrays> drawArrays = new osg::DrawArrays( osg::PrimitiveSet::LINES, 0, var_vertices->size() );
        var_geom->addPrimitiveSet(drawArrays.get());

        osg::ref_ptr<osg::Vec4Array> var_color = new osg::Vec4Array;
        var_color->push_back( osg::Vec4( 0.5, 0.1, 0.8, 1.0 ) );
        var_geom->setColorArray( var_color.get() );
        var_geom->setColorBinding( osg::Geometry::BIND_OVERALL );

        addDrawable( var_geom.get() );
    }
} // namespace vizkit3d
