#include <houio/HouScene.h>

/*
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStringList>
#include <QDebug>
#include <QVariantMap>
*/

HouScene::HouScene()
{
}


HouScene::~HouScene()
{
}

/*
base::Camera::Ptr HouScene::getCamera( const QString &name )
{
	auto it = m_cameras.find(name);
	if( it != m_cameras.end() )
		return it->second;
	return base::Camera::Ptr();
}

base::Camera::Ptr HouScene::loadCamera( QJsonObject &obj )
{
	base::Camera::Ptr camera = std::make_shared<base::Camera>();
	math::V2i res(int(obj.value("resx").toDouble()), int(obj.value("resy").toDouble()));
	camera->setRaster( res.x, res.y, float(res.x)/float(res.y) );

	math::M44f translation = math::M44f::TranslationMatrix(obj.value("transform.tx").toDouble(), obj.value("transform.ty").toDouble(), obj.value("transform.tz").toDouble());
	math::M44f rotationX = math::M44f::RotationMatrixX( -math::degToRad(obj.value("transform.rx").toDouble()) );
	math::M44f rotationY = math::M44f::RotationMatrixY( -math::degToRad(obj.value("transform.ry").toDouble()) );
	math::M44f rotationZ = math::M44f::RotationMatrixZ( -math::degToRad(obj.value("transform.rz").toDouble()) );

	camera->setViewToWorld( rotationX*rotationY*rotationZ*translation );
	return camera;
}


void HouScene::load( const QString &filename )
{
	// open new file
	QFile file;
	file.setFileName(filename);
	file.open(QIODevice::ReadOnly | QIODevice::Text);

	QJsonDocument sd = QJsonDocument::fromJson(file.readAll());
	QJsonObject root = sd.object();

	// cameras ---
	if( root.contains("cameras") )
	{
		QJsonObject cameras = root.value("cameras").toObject();
		for( auto it = cameras.begin(), end = cameras.end(); it != end; ++it )
			m_cameras[it.key()] = loadCamera( it.value().toObject() );
	}
}
*/
