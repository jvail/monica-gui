#ifndef QT_TOOLS_H_
#define QT_TOOLS_H_

#include <QtCore/QPair>
#include <QtCore/QRectF>
#include <QtCore/QPoint>
#include <QtGui/QColor>
#include <QtCore/QSizeF>

namespace Tools
{
	QColor desaturate(QColor c, double percentSaturation);

	/**
	 * gets the value of the given environment variable
	 * or an empty string if a variable with 'name' doesn't exist
	 */
	QString getEnvironmentVariable(const QString& name);

	/**
	 * return a rectangle resized with the correct aspect ratio, so that
	 * rectangle r1 fits exactly into rectangle r2
	 */
	QRectF fitRect1IntoRect2(const QRectF& r1, const QRectF& r2,
	                         QPair<int, int> r2ScaleInPercent =
	                        	 QPair<int, int>(100, 100));

	/**
	 * scales the given point according to the scaling used for the rectangle
	 * scaledRectangle when fitted into originalRect
	 */
  template<class Point>
  Point scalePointAccordingTo(QRectF originalRect, QRectF scaledRect, Point p)
  {
    double xFactor = scaledRect.width() / originalRect.width();
    double yFactor = scaledRect.height() / originalRect.height();

    return Point(int(p.x() * xFactor), int(p.y() * yFactor));
  }

	QPair<double, double>
	scaleFactorForFitRect1IntoRect2(const QRectF& r1,
	                                const QRectF& r2,
	                                QPair<int, int> r2ScaleInPercent =
	                                	QPair<int, int>(100, 100));

  QSizeF factorsToScaleRect1IntoRect2(QRectF r1, QRectF r2,
                                      QPair<int, int> r2ScaleInPercent =
                                      QPair<int, int>(100, 100));
}


#endif
