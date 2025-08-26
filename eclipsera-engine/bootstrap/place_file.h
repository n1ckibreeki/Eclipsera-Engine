const char place_script[] =
"-- Create the baseplate\n"
"local baseplate = Instance.new(\"Part\")\n"
"baseplate.Size = Vector3.new(200, 4, 200) -- X = 200, Y = 4, Z = 200\n"
"baseplate.Position = Vector3.new(0, 0, 0) -- sink half into the ground so the top is at Y=0\n"
"baseplate.Name = \"Baseplate\"\n"
"baseplate.Color = Color3.new(160,160,160)\n"
"baseplate.Anchored = true\n"
"baseplate.Parent = workspace\n";

const size_t place_script_len = sizeof(place_script) - 1;