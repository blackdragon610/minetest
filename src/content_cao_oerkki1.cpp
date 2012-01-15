/*
Minetest-c55
Copyright (C) 2010-2012 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "inventory.h"
#include "content_cao.h"

/*
	Oerkki1CAO
*/

class Oerkki1CAO : public ClientActiveObject
{
public:
	Oerkki1CAO(IGameDef *gamedef, ClientEnvironment *env):
		ClientActiveObject(0, gamedef, env),
		m_selection_box(-BS/3.,0.0,-BS/3., BS/3.,BS*2.,BS/3.),
		m_node(NULL),
		m_position(v3f(0,10*BS,0)),
		m_yaw(0),
		m_damage_visual_timer(0),
		m_damage_texture_enabled(false)
	{
		ClientActiveObject::registerType(getType(), create);
	}

	~Oerkki1CAO()
	{
	}

	inline u8 getType() const {	return ACTIVEOBJECT_TYPE_OERKKI1;	}

	static ClientActiveObject* create(IGameDef *gamedef, ClientEnvironment *env)
	{
		return new Oerkki1CAO(gamedef, env);
	}

	void addToScene(scene::ISceneManager *smgr, ITextureSource *tsrc,
				IrrlichtDevice *irr)
	{
		if(m_node != NULL)
			return;

		//video::IVideoDriver* driver = smgr->getVideoDriver();

		scene::SMesh *mesh = new scene::SMesh();
		scene::IMeshBuffer *buf = new scene::SMeshBuffer();
		video::SColor c(255,255,255,255);
		video::S3DVertex vertices[4] =
		{
			video::S3DVertex(-BS/2-BS,0,0, 0,0,0, c, 0,1),
			video::S3DVertex(BS/2+BS,0,0, 0,0,0, c, 1,1),
			video::S3DVertex(BS/2+BS,BS*2,0, 0,0,0, c, 1,0),
			video::S3DVertex(-BS/2-BS,BS*2,0, 0,0,0, c, 0,0),
		};
		u16 indices[] = {0,1,2,2,3,0};
		buf->append(vertices, 4, indices, 6);
		// Set material
		buf->getMaterial().setFlag(video::EMF_LIGHTING, false);
		buf->getMaterial().setFlag(video::EMF_BACK_FACE_CULLING, false);
		//buf->getMaterial().setTexture(0, NULL);
		buf->getMaterial().setTexture(0, tsrc->getTextureRaw("oerkki1.png"));
		buf->getMaterial().setFlag(video::EMF_BILINEAR_FILTER, false);
		buf->getMaterial().setFlag(video::EMF_FOG_ENABLE, true);
		buf->getMaterial().MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
		// Add to mesh
		mesh->addMeshBuffer(buf);
		buf->drop();
		m_node = smgr->addMeshSceneNode(mesh, NULL);
		mesh->drop();
		// Set it to use the materials of the meshbuffers directly.
		// This is needed for changing the texture in the future
		m_node->setReadOnlyMaterials(true);
		updateNodePos();
	}

	void removeFromScene()
	{
		if(m_node == NULL)
			return;

		m_node->remove();
		m_node = NULL;
	}

	void updateLight(u8 light_at_pos)
	{
		if(m_node == NULL)
			return;

		if(light_at_pos <= 2)
		{
			m_node->setVisible(false);
			return;
		}

		m_node->setVisible(true);

		u8 li = decode_light(light_at_pos);
		video::SColor color(255,li,li,li);
		setMeshColor(m_node->getMesh(), color);
	}

	v3s16 getLightPosition()
	{
		return floatToInt(m_position+v3f(0,BS*1.5,0), BS);
	}

	void updateNodePos()
	{
		if(m_node == NULL)
			return;

		//m_node->setPosition(m_position);
		m_node->setPosition(pos_translator.vect_show);

		v3f rot = m_node->getRotation();
		rot.Y = 180.0 - m_yaw + 90.0;
		m_node->setRotation(rot);
	}

	void step(float dtime, ClientEnvironment *env)
	{
		ITextureSource *tsrc = m_gamedef->tsrc();

		pos_translator.translate(dtime);
		updateNodePos();

		LocalPlayer *player = env->getLocalPlayer();
		assert(player);

		v3f playerpos = player->getPosition();
		v2f playerpos_2d(playerpos.X,playerpos.Z);
		v2f objectpos_2d(m_position.X,m_position.Z);

		if(fabs(m_position.Y - playerpos.Y) < 1.5*BS &&
				objectpos_2d.getDistanceFrom(playerpos_2d) < 1.5*BS)
		{
			if(m_attack_interval.step(dtime, 0.5))
			{
				env->damageLocalPlayer(2);
			}
		}

		if(m_damage_visual_timer > 0)
		{
			if(!m_damage_texture_enabled)
			{
				// Enable damage texture
				if(m_node)
				{
					/*video::IVideoDriver* driver =
						m_node->getSceneManager()->getVideoDriver();*/

					scene::IMesh *mesh = m_node->getMesh();
					if(mesh == NULL)
						return;

					u16 mc = mesh->getMeshBufferCount();
					for(u16 j=0; j<mc; j++)
					{
						scene::IMeshBuffer *buf = mesh->getMeshBuffer(j);
						buf->getMaterial().setTexture(0,
								tsrc->getTextureRaw("oerkki1_damaged.png"));
					}
				}
				m_damage_texture_enabled = true;
			}
			m_damage_visual_timer -= dtime;
		}
		else
		{
			if(m_damage_texture_enabled)
			{
				// Disable damage texture
				if(m_node)
				{
					/*video::IVideoDriver* driver =
						m_node->getSceneManager()->getVideoDriver();*/

					scene::IMesh *mesh = m_node->getMesh();
					if(mesh == NULL)
						return;

					u16 mc = mesh->getMeshBufferCount();
					for(u16 j=0; j<mc; j++)
					{
						scene::IMeshBuffer *buf = mesh->getMeshBuffer(j);
						buf->getMaterial().setTexture(0,
								tsrc->getTextureRaw("oerkki1.png"));
					}
				}
				m_damage_texture_enabled = false;
			}
		}
	}

	void processMessage(const std::string &data)
	{
		//infostream<<"Oerkki1CAO: Got message"<<std::endl;
		std::istringstream is(data, std::ios::binary);
		// command
		u8 cmd = readU8(is);
		if(cmd == 0)
		{
			// pos
			m_position = readV3F1000(is);
			pos_translator.update(m_position);
			// yaw
			m_yaw = readF1000(is);
			updateNodePos();
		}
		else if(cmd == 1)
		{
			//u16 damage = readU8(is);
			m_damage_visual_timer = 1.0;
		}
	}

	void initialize(const std::string &data)
	{
		//infostream<<"Oerkki1CAO: Got init data"<<std::endl;

		{
			std::istringstream is(data, std::ios::binary);
			// version
			u8 version = readU8(is);
			// check version
			if(version != 0)
				return;
			// pos
			m_position = readV3F1000(is);
			pos_translator.init(m_position);
		}

		updateNodePos();
	}

	core::aabbox3d<f32>* getSelectionBox()
		{return &m_selection_box;}

	v3f getPosition()
		{return pos_translator.vect_show;}

	bool directReportPunch(const std::string &toolname, v3f dir)
	{
		m_damage_visual_timer = 1.0;

		m_position += dir * BS;
		pos_translator.sharpen();
		pos_translator.update(m_position);
		updateNodePos();

		return false;
	}

private:
	IntervalLimiter m_attack_interval;
	core::aabbox3d<f32> m_selection_box;
	scene::IMeshSceneNode *m_node;
	v3f m_position;
	float m_yaw;
	SmoothTranslator pos_translator;
	float m_damage_visual_timer;
	bool m_damage_texture_enabled;
};

// Prototype
Oerkki1CAO proto_Oerkki1CAO(NULL, NULL);
