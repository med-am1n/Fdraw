#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos; 


uniform vec3 viewPos;
uniform	sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform	sampler2D texture_normal0;
uniform	sampler2D texture_height0;
uniform float shininess;


struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
uniform DirLight dirLight;
vec4 calculateDirectLight(){
	// ambient
    vec3 ambient = dirLight.ambient * texture(texture_diffuse0, TexCoord).rgb;
	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(-dirLight.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = dirLight.diffuse * diff * texture(texture_diffuse0, TexCoord).rgb;
	// specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = dirLight.specular * spec * vec3(texture(texture_specular0, TexCoord));

	return vec4(ambient + diffuse + specular, 1.0);
}

struct PointLight {    
    vec3 position;
    
    float a;
    float b;
    float c;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}; 
uniform PointLight pointLight;
vec4 calculatePointLight(){

	vec3 lightVec=pointLight.position - FragPos;
	float dist = length(lightVec);
	float inten = 1.0f / (pointLight.a * dist * dist + pointLight.b * dist + pointLight.c);
	// ambient
    vec3 ambient = pointLight.ambient * texture(texture_diffuse0, TexCoord).rgb;
	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightVec);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = pointLight.diffuse * diff * texture(texture_diffuse0, TexCoord).rgb;
	// specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(viewDir, -halfwayDir), 0.0), shininess);
	vec3 specular = pointLight.specular * spec * vec3(texture(texture_specular0, TexCoord).r);

	return vec4(ambient + diffuse * inten + specular * inten, 1.0);
}

struct SpotLight{
	vec3 position;
	vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLight;
vec4 calculateSpotLight(){

	float outerCone = 0.90f;
	float innerCone = 0.95f;
	// ambient
    vec3 ambient = spotLight.ambient * texture(texture_diffuse0, TexCoord).rgb;
	// diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(spotLight.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = spotLight.diffuse * diff * texture(texture_diffuse0, TexCoord).rgb;
	// specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = spotLight.specular * spec * vec3(texture(texture_specular0, TexCoord));

	float angle = dot(lightDir, normalize(-spotLight.direction));
	float epsilon = innerCone - outerCone;
	float inten = clamp((angle - outerCone)/ epsilon, 0.0f, 1.0f);
	return vec4(ambient + diffuse * inten + specular * inten, 1.0);
}

void main()
{	
	vec3 color = texture(texture_diffuse0, TexCoord).rgb;
    FragColor = vec4(color, 1.0);
}