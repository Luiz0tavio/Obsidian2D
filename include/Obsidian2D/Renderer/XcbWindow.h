#ifndef OBSIDIAN2D_RENDERER_XCBWINDOW_
#define OBSIDIAN2D_RENDERER_XCBWINDOW_

#define XYZ1(_x_, _y_, _z_) (_x_), (_y_), (_z_), 1.0

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <bitset>
#include <xcb/xcb.h>

#include "Obsidian2D/Renderer/Window.h"
#include "Obsidian2D/Core/WindowEvent.h"

struct Vertex {
	float posX, posY, posZ, posW;  // Position data
	float r, g, b, a;              // Color
};

static const VertexUV g_vb_texture_Data[] = {
		{XYZ1(-1, -1, 1)},
		{XYZ1( 1, -1, 1)},
		{XYZ1( 1,  1, 1)},
		{XYZ1(-1, -1, 1)},
		{XYZ1( 1,  1, 1)},
		{XYZ1(-1,  1, 1)}
};

namespace Obsidian2D
{
	namespace Renderer
	{
		class XcbWindow: public Window
		{
		public:

			~XcbWindow(){

				vkDestroySurfaceKHR(instance, surface, NULL);
				xcb_destroy_window(connection, window);
				xcb_disconnect(connection);

			}
		private:

			xcb_screen_t* 							screen;
			xcb_window_t 							window;
			xcb_connection_t*						connection;

			void setConnection()
			{
				const xcb_setup_t *setup;
				xcb_screen_iterator_t iter;
				int scr;

				connection = xcb_connect(NULL, &scr);
				if (connection == NULL || xcb_connection_has_error(connection)) {
					std::cout << "Unable to make an XCB connection\n";
					exit(-1);
				}

				setup = xcb_get_setup(connection);
				iter = xcb_setup_roots_iterator(setup);
				while (scr-- > 0) xcb_screen_next(&iter);

				screen = iter.data;
			}

			void createWindow()
			{
				assert(this->width > 0);
				assert(this->height > 0);

				uint32_t value_mask, value_list[32];

				window = xcb_generate_id(connection);

				value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
				value_list[0] = screen->black_pixel;
				value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS |
								XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW;

				xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root,
								  0, 0, (uint16_t)this->width, (uint16_t)this->height, 0,
								  XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, value_list);

				/* Magic code that will send notification when window is destroyed */
				xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 1, 12, "WM_PROTOCOLS");
				xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, 0);

				xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
				xcb_intern_atom_reply_t *atom_wm_delete_window = xcb_intern_atom_reply(connection, cookie2, 0);

				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, (*reply).atom, 4, 32, 1,
									&(*atom_wm_delete_window).atom);
				free(reply);

				xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window,
									XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(APP_NAME), APP_NAME);

				xcb_flush(connection);

				xcb_map_window(connection, window);

				// Force the x/y coordinates to 100, 100 results are identical in consecutive
				// runs
				const uint32_t coords[] = {100, 100};
				xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
				xcb_flush(connection);
			}

			void createSurface()
			{
				VkXcbSurfaceCreateInfoKHR createInfo = {};
				createInfo.sType 						= VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
				createInfo.pNext 						= nullptr;
				createInfo.connection 					= connection;
				createInfo.window 						= window;

				VkResult res = vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &surface);

				assert(res == VK_SUCCESS);
			}

		public:
			XcbWindow(int32_t width, int32_t height)
			{
				this->width = width;
				this->height = height;
			}

			void bootstrap()
			{
				const bool depthPresent = true;

				this->createInstance();
				this->createLogicalDeviceAndCommandBuffer();
				this->setConnection();
				this->createWindow();
				this->createSurface();
				this->initGraphicPipeline(depthPresent, g_vb_texture_Data);
				this->draw();
			}

			int y = 0, x =0;
			int c_x = 0, c_y = 0;

			::Obsidian2D::Core::WindowEvent poolEvent()
			{
				xcb_generic_event_t* e = nullptr;
				//xcb_intern_atom_reply_t *atom_wm_delete_window = nullptr;

				while ((e = xcb_poll_for_event(connection))) {
					if ((e->response_type & ~0x80) == XCB_CLIENT_MESSAGE) {
						//if((*(xcb_client_message_event_t*)e).data.data32[0] == (*atom_wm_delete_window).atom) {
							return ::Obsidian2D::Core::WindowEvent::Close;
						//}
					} else if((e->response_type & ~0x80) == XCB_ENTER_NOTIFY) {
						return ::Obsidian2D::Core::WindowEvent::Focus;
					} else if((e->response_type & ~0x80) == XCB_LEAVE_NOTIFY) {
						return ::Obsidian2D::Core::WindowEvent::Blur;
					} else if((e->response_type & ~0x80) == XCB_BUTTON_PRESS) {
						return ::Obsidian2D::Core::WindowEvent::Click;
					} else if((e->response_type & ~0x80) == XCB_BUTTON_RELEASE) {
						//return ::Obsidian2D::Core::WindowEvent::ClickEnd;
					} else if((e->response_type & ~0x80) == XCB_KEY_PRESS) {

						xcb_key_press_event_t * kp = (xcb_key_press_event_t *)e;

						if( kp->detail == 'O'){
							x--;
						}else if(kp->detail == 'P'){
							y++;
						}else if(kp->detail == 'Q'){
							x++;
						}else if(kp->detail == 'j'){
							y--;
						}

						if( kp->detail == 'W'){
							c_x--;
						}else if(kp->detail == 'X'){
							c_y--;
						}else if(kp->detail == 'Y'){
							c_x++;
						}else if(kp->detail == 'T'){
							c_y++;
						}

						std::cout << kp->detail << std::endl;
						this->setCameraViewCenter(glm::vec3(x, y, 0));
						this->setCameraViewEye(glm::vec3(c_x, c_y, -10));
						this->updateCamera();

						this->draw();

						return ::Obsidian2D::Core::WindowEvent::ButtonDown;
					} else if((e->response_type & ~0x80) == XCB_KEY_RELEASE) {
						return ::Obsidian2D::Core::WindowEvent::ButtonUp;
					} else {
						return ::Obsidian2D::Core::WindowEvent::Unknow;
					}
				}

				return ::Obsidian2D::Core::WindowEvent::None;
			}
		};
	}
}

#endif //OBSIDIAN2D_RENDERER_XCBWINDOW_
