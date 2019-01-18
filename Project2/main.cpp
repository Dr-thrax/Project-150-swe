#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <string> 
#include <cstdlib>



using namespace std;
using namespace sf;

 constexpr float ballRadius{ 10.0f }, ballVelocity{ 8.f };
constexpr int WindowHeight{ 720 }, WindowWidth{ 950 };
constexpr  float paddleWidth{ 120.0f }, paddleHeight{ 10.0f }, paddleVelocity{16.f};
constexpr float blockWidth{ 60.f }, blockHeight{ 20.f };
constexpr int countBlocksx{ 14 }, countBlocksy{ 4 };
int score{ 0 };

Music music;
Music music2;

Texture paddleTexture;
Texture BallTexture;
Texture brickTexture;
Texture loading;
Texture texture;

Sprite sprite;


Text points;
Font font;


struct Ball
{
	CircleShape shape;

	Vector2f velocity{ -ballVelocity,-ballVelocity };

	Ball(float mx, float my)
	{
		shape.setPosition(mx, my);
		shape.setRadius(ballRadius);
		shape.setFillColor(Color::White);
		shape.setOrigin(ballRadius, ballRadius);

	}

	void update() 
	{
		shape.move(velocity);

		if (left() < 0) velocity.x = ballVelocity;
		else if (right() > WindowWidth) velocity.x = -ballVelocity;
		if (top() < 0) velocity.y = ballVelocity;
		else if (bottom() > WindowHeight) velocity.y = -ballVelocity;

		
	}

	float x() { return shape.getPosition().x; }
	float y() { return shape.getPosition().y; }
	float left() { return x() - shape.getRadius(); }
	float right() { return x() + shape.getRadius(); }
	float top() { return y() - shape.getRadius(); }
	float bottom() { return y() + shape.getRadius(); }
};

struct paddle
{
	RectangleShape shape;
	Vector2f velocity;

	paddle(float mx, float my)
	{
		shape.setPosition(mx, my);
		shape.setSize({ paddleWidth,paddleHeight });
		shape.setFillColor(Color::White);
		shape.setOrigin(paddleWidth / 2.f, paddleHeight / 2.f);
	}

	float x() { return shape.getPosition().x; }
	float y() { return shape.getPosition().y; }
	float left() { return x()   -  shape.getSize().x / 2.0f; }
	float right() { return x()  +  shape.getSize().x / 2.0f; }
	float top() { return y()    -  shape.getSize().y / 2.0f; }
	float bottom() { return y() +  shape.getSize().y / 2.0f; }

	void update()
	{
		shape.move(velocity);


		if (Keyboard::isKeyPressed(Keyboard::Key::Left) && left() > 0) velocity.x = -paddleVelocity;
		else if (Keyboard::isKeyPressed(Keyboard::Key::Right) && right() < WindowWidth) velocity.x = paddleVelocity;
		else velocity.x = 0;
 	}
};


struct Brick 
{
	RectangleShape shape;

	bool destroyed{ false };

	Brick(float mX, float mY)
	{
		shape.setPosition(mX, mY);
		shape.setSize({ blockWidth, blockHeight });
		
		shape.setOrigin(blockWidth / 2.f, blockHeight / 2.f);

	}

	float x() { return shape.getPosition().x; }
	float y() { return shape.getPosition().y; }
	float left() { return x() - shape.getSize().x / 2.0f; }
	float right() { return x() + shape.getSize().x / 2.0f; }
	float top() { return y() - shape.getSize().y / 2.0f; }
	float bottom() { return y() + shape.getSize().y / 2.0f; }



};

template<class T1, class T2> bool isIntersecting(T1& mA, T2& mB)
{
	return mA.right() >= mB.left() && mA.left() <= mB.right() 
		&& mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}


void testCollision(paddle& mpaddle, Ball& mBall)
{
	if (!isIntersecting(mpaddle, mBall)) return;

	mBall.velocity.y = -ballVelocity;
	music2.openFromFile("boink3.wav");
	music2.setVolume(100);
	music2.play();

	if (mBall.x() < mpaddle.x()) mBall.velocity.x = -ballVelocity;
	else mBall.velocity.x = ballVelocity;
}


void testCollision(Brick& mBrick, Ball& mBall)
{
	if (!isIntersecting(mBrick, mBall)) return;

	mBrick.destroyed = true;
	score += 150;
	music.openFromFile("boink2.wav");
	music.setVolume(100);
	music.play();
		
	float overlapLeft{ mBall.right() - mBrick.left() };
	float overlapRight{ mBrick.right() - mBall.left() };
	float overlapTop{ mBall.bottom() - mBrick.top() };
	float overlapBottom{ mBrick.bottom() - mBall.top() };
		
	bool ballfromLeft(abs(overlapLeft) < abs(overlapRight));
	bool ballfromTop(abs(overlapTop) < abs(overlapBottom));
	
	float minOverlapX{ ballfromLeft ? overlapLeft : overlapRight };
	float minOverlapY{ ballfromTop ? overlapTop : overlapBottom };

	if (abs(minOverlapX) < abs(minOverlapY)) 
	{
		mBall.velocity.x = ballfromLeft ? -ballVelocity : ballVelocity;
	}

	else
	{
		mBall.velocity.y = ballfromTop ? -ballVelocity : ballVelocity;
	}


}

class ParticleSystem : public sf::Drawable, public sf::Transformable
{
public:

	ParticleSystem(unsigned int count) :
	m_particles(count),
	m_vertices(sf::Points, count),
	m_lifetime(sf::seconds(3.f)),
	m_emitter(0.f, 0.f)
	{
	}

	void setEmitter(sf::Vector2f position)
	{
		m_emitter = position;
	}

	void update(sf::Time elapsed)
	{
		for (std::size_t i = 0; i < m_particles.size(); ++i)
		{
			// update the particle lifetime
			Particle& p = m_particles[i];
			p.lifetime -= elapsed;

			// if the particle is dead, respawn it
			if (p.lifetime <= sf::Time::Zero)
				resetParticle(i);

			// update the position of the corresponding vertex
			m_vertices[i].position += p.velocity * elapsed.asSeconds();

			// update the alpha (transparency) of the particle according to its lifetime
			float ratio = p.lifetime.asSeconds() / m_lifetime.asSeconds();
			m_vertices[i].color.a = static_cast<sf::Uint8>(ratio * 255);
		}
	}

private:

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		// apply the transform
		states.transform *= getTransform();

		// our particles don't use a texture
		states.texture = NULL;

		// draw the vertex array
		target.draw(m_vertices, states);
	}

private:

	struct Particle
	{
		sf::Vector2f velocity;
		sf::Time lifetime;
	};

	void resetParticle(std::size_t index)
	{
		// give a random velocity and lifetime to the particle
		float angle = (std::rand() % 360) * 3.14f / 180.f;
		float speed = (std::rand() % 50) + 50.f;
		m_particles[index].velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
		m_particles[index].lifetime = sf::milliseconds((std::rand() % 1000) );

		// reset the position of the corresponding vertex
		m_vertices[index].position = m_emitter;
	}

	std::vector<Particle> m_particles;
	sf::VertexArray m_vertices;
	sf::Time m_lifetime;
	sf::Vector2f m_emitter;
};
int main()
{
	sf::RenderWindow window(VideoMode(WindowWidth+250, WindowHeight), "Break The Bricks");
	window.setFramerateLimit(60);

	Ball ball{ WindowWidth / 2,WindowHeight / 2 };
	paddle Paddle{ WindowWidth / 2.f,WindowHeight - 5.f};

	vector <Brick> bricks;

	for (int iX{ 0 }; iX < countBlocksx; ++iX)
		for (int iY{ 0 }; iY < countBlocksy; ++iY)
			bricks.emplace_back((iX + 1) * (blockWidth + 3) + 22, (iY + 2) * (blockHeight + 3));

	Music game_music;
	game_music.openFromFile("SONG01.ogg");
	game_music.setVolume(35);
	game_music.play();
	loading.loadFromFile("4.png");

	RectangleShape loadingscreen;
	
	loadingscreen.setTexture(&loading);
	loadingscreen.setFillColor(Color::White);
	loadingscreen.setScale({ 1,1 });
	loadingscreen.setSize({ 100,100 });
	loadingscreen.setPosition(1070, 190);
	loadingscreen.setOrigin({ 50, 50 });
	

	


	


	BallTexture.loadFromFile("ball.png");
	brickTexture.loadFromFile("goldbrick.png");
	texture.loadFromFile("score.png");
	font.loadFromFile("Impacted.ttf");
	Vector2u size = texture.getSize();
	sprite.setTexture(texture);
	ParticleSystem particles(1000);
	sf::Clock clock;

	while (true)
	{
		 
		game_music.setLoop(true);
		window.clear(Color::Black);
		
		if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

		sf::Vector2i mouse;
		mouse.x = (int) ball.shape.getPosition().x;
		mouse.y = (int)ball.shape.getPosition().y;
		particles.setEmitter(window.mapPixelToCoords(mouse));



		ball.update();
		Paddle.update();
		testCollision(Paddle, ball);
		paddleTexture.loadFromFile("paddle.png");

		points.setFont(font);
		points.setStyle(Text::Bold);
		points.StrikeThrough;
		points.setOutlineColor(Color::Black);
		points.setCharacterSize(60);
		points.setOutlineThickness(10);
		points.setPosition(1015, 460);
		std::stringstream ss;
		ss <<  score;
		points.setString(ss.str());

		sf::Time elapsed = clock.restart();
		particles.update(elapsed);
		
		loadingscreen.rotate(4);

		Paddle.shape.setTexture(&paddleTexture);
		ball.shape.setTexture(&BallTexture);
		sprite.setPosition(WindowWidth , 0);
		
		
		

		for (auto& brick : bricks) testCollision(brick, ball);
		bricks.erase(remove_if(begin(bricks), end(bricks), [](const Brick& mBrick) { return mBrick.destroyed; }),
			end(bricks));

		for (auto& brick : bricks) brick.shape.setTexture(&brickTexture);
			
		

		window.draw(ball.shape);
		window.draw(Paddle.shape);
		for (auto& brick : bricks) window.draw(brick.shape);
		window.draw(sprite);
		window.draw(points);
		window.draw(loadingscreen);
		window.draw(particles);
		window.display();
	}

	return 0;


}