"Scheme"
{
	"BaseSettings"
	{
		"Panel.Title.OffsetX"							"0"
		"Panel.Title.OffsetY"							"0"

		"Panel.Button.Width.Out"						"256"
		"Panel.Button.Width.Over"						"256"
		"Panel.Button.Width.Pressed"					"256"
		"Panel.Button.Width.Released"					"256"

		"Panel.Button.Height.Out"						"64"
		"Panel.Button.Height.Over"						"64"
		"Panel.Button.Height.Pressed"					"64"
		"Panel.Button.Height.Released"					"64"

		"Panel.Button.Text.OffsetX"						"24"
		"Panel.Button.Text.OffsetY"						"0"

		"Panel.Button.Description.OffsetX"				"1"
		"Panel.Button.Description.OffsetY"				"-3"

		"Panel.Button.Description.Hide.Out"				"1"
		"Panel.Button.Description.Hide.Over"			"0"
		"Panel.Button.Description.Hide.Pressed"			"0"
		"Panel.Button.Description.Hide.Released"		"0"

		"Panel.Button.Background.Blur.Out"				"0"
		"Panel.Button.Background.Blur.Over"				"1"
		"Panel.Button.Background.Blur.Pressed"			"1"
		"Panel.Button.Background.Blur.Released"			"1"

		"Panel.Button.Animation.Width"					"0.15"
		"Panel.Button.Animation.Height"					"0.25"
		"Panel.Button.Animation.Background"				"0.2"
		"Panel.Button.Animation.Text"					"0.2"
		"Panel.Button.Animation.Description"			"0.3"
		
		//Sounds
		"Button.Sound.Armed"							"ui/button_over.wav"
		"Button.Sound.Released"							"ui/button_release.wav"
		"Button.Sound.Depressed"						"ui/button_click.wav"
	}

	"Colors"
	{
		"Panel.Background.Gradient.Top"					"0 0 0 0"
		"Panel.Background.Gradient.Bottom"				"0 0 0 255"

		"Panel.Title.Color"								"255 255 255 255"
		
		"Panel.Button.Background.Out"					"0 0 0 0"
		"Panel.Button.Background.Over"					"255 255 255 3"
		"Panel.Button.Background.Pressed"				"255 255 255 255"
		"Panel.Button.Background.Released"				"0 0 0 255"

		"Panel.Button.Background.Outline.Out"			"0 0 0 0"
		"Panel.Button.Background.Outline.Over"			"0 0 0 0"
		"Panel.Button.Background.Outline.Pressed"		"0 0 0 0"
		"Panel.Button.Background.Outline.Released"		"0 0 0 0"

		"Panel.Button.Text.Out"							"255 255 255 150"
		"Panel.Button.Text.Over"						"150 150 150 255"
		"Panel.Button.Text.Pressed"						"0 0 0 255"
		"Panel.Button.Text.Released"					"255 255 255 100"

		"Panel.Button.Description.Out"					"0 0 0 0"
		"Panel.Button.Description.Over"					"180 180 180 200"
		"Panel.Button.Description.Pressed"				"180 180 180 200"
		"Panel.Button.Description.Released"				"180 180 180 200"
	}

	"Fonts"
	{
		"Panel.Title.Font"
		{
			"1"
			{
				"name"			"GorDIN Bold"
				"tall"			"64"
				"weight"		"0"
				"antialias"		"1"
			}
		}

		"Panel.Button.Text.Font"
		{
			"1"
			{
				"name"			"GorDIN Light"
				"tall"			"32"
				"weight"		"600"
				"antialias"		"1"
			}
		}

		"Panel.Button.Description.Font"
		{
			"1"
			{
				"name"			"GorDIN Light"
				"tall"			"24"
				"weight"		"600"
				"antialias"		"1"
			}
		}
	}

	"CustomFontFiles"
	{
		"file"		"gameui2/fonts/gordin-bold.ttf"
		"file"		"gameui2/fonts/gordin-regular.ttf"
		"file"		"gameui2/fonts/gordin-light.ttf"
	}
}