vec3 ProcessPointLight(
	vec3 diffuseColor,
	vec3 specularColor,
	vec3 fragPos,
	vec3 lightPos,
	vec3 lightColor,
	vec3 viewPos,
	vec3 normal,
	int index)
{
	vec3 ambient = vec3(0.05f) * lightColor;

	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = lightColor * diff;

	float specularStrength = 0.5;
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	float distance = length(lightPos - fragPos);
	float attenuation = 1.0f / (1.0f + 0.09f * distance +
		0.32f * (distance * distance));

	float shadow = CalculateShadow(fragPos, viewPos, index);
	return (ambient * diffuseColor +
		(diffuse * diffuseColor + specular * specularColor)
		* (1.0 - shadow)) * attenuation;
}

vec3 ProcessSpotLight(
	vec3 diffuseColor,
	vec3 specularColor,
	vec3 fragPos,
	vec3 lightPos,
	vec3 lightColor,
	vec3 lightOrient,
	vec3 viewPos,
	vec3 normal,
	float lightAngle,
	float lightOuterAngle,
	int index)
{
	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	vec3 ambient = vec3(0.05f) * lightColor;

	float distance = length(lightPos - fragPos);
	float attenuation = 1.0f / (1.0f + 0.09f * distance +
		0.32f * (distance * distance));

	float theta = dot(lightDir, normalize(-lightOrient));
	float epsilon = lightAngle - lightOuterAngle;
	float intensity = clamp((theta - lightOuterAngle) /
		epsilon, 0.0, 1.0);

	float shadow = CalculateShadow(fragPos, viewPos, index);

	if (theta > lightOuterAngle) {
		float diff = max(dot(normal, lightDir), 0.0);
		vec3 diffuse = lightColor * diff;

		float specularStrength = 0.5;
		float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
		vec3 specular = specularStrength * spec * lightColor;

		return (ambient * diffuseColor + (diffuse * diffuseColor +
			specular * specularColor) * intensity *
			(1.0 - shadow)) * attenuation;
	} else {
		return ambient * diffuseColor * attenuation;
	}
}
