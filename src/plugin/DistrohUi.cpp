#include "DistrhoUI.hpp"
#include "Color.hpp"

START_NAMESPACE_DISTRHO

using DGL_NAMESPACE::Color;
using DGL_NAMESPACE::GraphicsContext;
using DGL_NAMESPACE::Rectangle;

// -----------------------------------------------------------------------------------------------------------

class DistrohUi : public UI {
public:
	/**
	  Get key name from an index.
	*/
	static const char* getStateKeyFromIndex(const uint32_t index) noexcept {
		switch (index)
		{
		case 0: return "top-left";
		case 1: return "top-center";
		case 2: return "top-right";
		case 3: return "middle-left";
		case 4: return "middle-center";
		case 5: return "middle-right";
		case 6: return "bottom-left";
		case 7: return "bottom-center";
		case 8: return "bottom-right";
		}

		return "unknown";
	}

	/* constructor */
	DistrohUi() : UI(512, 512) {
	   /**
		  Initialize the grid to all off per default.
		*/
		std::memset(fParamGrid, 0, sizeof(bool)*9);

		// TODO explain why this is here
		setGeometryConstraints(128, 128, true);
	}

protected:
   /* --------------------------------------------------------------------------------------------------------
	* DSP/Plugin Callbacks */

	void parameterChanged(uint32_t index, float value) override {
		repaint();
	}


   /**
	  A state has changed on the plugin side.
	  This is called by the host to inform the UI about state changes.
	*/
	void stateChanged(const char* key, const char* value) override {
		if (std::strcmp(key, DISTRHO_PLUGIN_STATE_PATCH) == 0) {
			// load UI?
			// cardinal doesn't
		}

		if (std::strcmp(key, "windowSize") != 0) { return; }

		int width = 0;
		int height = 0;
		std::sscanf(value, "%i:%i", &width, &height);

		if (width > 0 && height > 0) {
			const double scaleFactor = getScaleFactor();
			setSize(width * scaleFactor, height * scaleFactor);
		}
	}

   /* --------------------------------------------------------------------------------------------------------
	* Widget Callbacks */

   /**
	  The OpenGL drawing function.
	  This UI will draw a 3x3 grid, with on/off states according to plugin state.
	*/
	void onDisplay() override {
		const GraphicsContext& context(getGraphicsContext());

		const uint width = getWidth();
		const uint height = getHeight();
		const uint minwh = std::min(width, height);
		const uint bgColor = getBackgroundColor();

		Rectangle<double> r;

		// if host doesn't respect aspect-ratio but supports ui background, draw out-of-bounds color from it
		if (width != height && bgColor != 0)
		{
			const int red   = (bgColor >> 24) & 0xff;
			const int green = (bgColor >> 16) & 0xff;
			const int blue  = (bgColor >>  8) & 0xff;
			Color(red, green, blue).setFor(context);

			if (width > height)
			{
				r.setPos(height, 0);
				r.setSize(width-height, height);
			}
			else
			{
				r.setPos(0, width);
				r.setSize(width, height-width);
			}

			r.draw(context);
		}

		r.setWidth(minwh/3 - 6);
		r.setHeight(minwh/3 - 6);

		// draw left, center and right columns
		for (int i=0; i<3; ++i)
		{
			r.setX(3 + i*minwh/3);

			// top
			r.setY(3);

			if (fParamGrid[0+i])
				Color(0.8f, 0.5f, 0.3f).setFor(context);
			else
				Color(0.3f, 0.5f, 0.8f).setFor(context);

			r.draw(context);

			// middle
			r.setY(3 + minwh/3);

			if (fParamGrid[3+i])
				Color(0.8f, 0.5f, 0.3f).setFor(context);
			else
				Color(0.3f, 0.5f, 0.8f).setFor(context);

			r.draw(context);

			// bottom
			r.setY(3 + minwh*2/3);

			if (fParamGrid[6+i])
				Color(0.8f, 0.5f, 0.3f).setFor(context);
			else
				Color(0.3f, 0.5f, 0.8f).setFor(context);

			r.draw(context);
		}
	}

   /**
	  Mouse press event.
	  This UI will de/activate blocks when you click them and report it as a state change to the plugin.
	*/
	bool onMouse(const MouseEvent& ev) override {
		// Test for left-clicked + pressed first.
		if (ev.button != 1 || ! ev.press)
			return false;

		const uint width = getWidth();
		const uint height = getHeight();

		Rectangle<double> r;

		r.setWidth(width/3 - 6);
		r.setHeight(height/3 - 6);

		// handle left, center and right columns
		for (int i=0; i<3; ++i)
		{
			r.setX(3 + i*width/3);

			// top
			r.setY(3);

			if (r.contains(ev.pos))
			{
				// index that this block applies to
				const uint32_t index = 0+i;

				// invert block state
				fParamGrid[index] = !fParamGrid[index];

				// report change to host (and thus plugin)
				setState(getStateKeyFromIndex(index), fParamGrid[index] ? "true" : "false");

				// trigger repaint
				repaint();
				break;
			}

			// middle
			r.setY(3 + height/3);

			if (r.contains(ev.pos))
			{
				// same as before
				const uint32_t index = 3+i;
				fParamGrid[index] = !fParamGrid[index];
				setState(getStateKeyFromIndex(index), fParamGrid[index] ? "true" : "false");
				repaint();
				break;
			}

			// bottom
			r.setY(3 + height*2/3);

			if (r.contains(ev.pos))
			{
				// same as before
				const uint32_t index = 6+i;
				fParamGrid[index] = !fParamGrid[index];
				setState(getStateKeyFromIndex(index), fParamGrid[index] ? "true" : "false");
				repaint();
				break;
			}
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------

private:
   /**
	  Our states used to display the grid.
	  The host does not know about these.
	*/
	bool fParamGrid[9];

   /**
	  Set our UI class as non-copyable and add a leak detector just in case.
	*/
	DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistrohUi)
};

UI* createUI() { return new DistrohUi(); }

END_NAMESPACE_DISTRHO