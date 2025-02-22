#version 450 core

/*
 * Copyright (c) 2018-2019 Confetti Interactive Inc.
 * 
 * This file is part of The-Forge
 * (see https://github.com/ConfettiFX/The-Forge).
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
*/


layout (UPDATE_FREQ_NONE, binding=0) uniform texture2D  uTex0;
layout (UPDATE_FREQ_NONE, binding=1) uniform sampler   uSampler0;

layout(location = 0) in vec2 texcoord;

layout(location = 0) out vec4 fs_out_color;

void main(void)
{
	vec4 samp = texture(sampler2D(uTex0, uSampler0), texcoord);
	fs_out_color = vec4(samp);
}
