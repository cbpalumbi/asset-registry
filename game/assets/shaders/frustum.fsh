#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

// Frustum parameters in screen space
uniform vec2 frustumOrigin;
uniform vec2 frustumDirection;
uniform float frustumHalfAngle;  // radians
uniform float frustumRange;
uniform vec2 resolution;

void main() {
    vec4 texColor = texture(texture0, fragTexCoord) * fragColor;

    // Convert from UV space to screen space
    vec2 screenPos = fragTexCoord * resolution;

    vec2 toFragment = screenPos - frustumOrigin;
    float dist = length(toFragment);

    bool inFrustum = false;
    if (dist < 0.001) {
        inFrustum = true;
    } else {
        vec2 toFragNorm = toFragment / dist;
        float cosHalf = cos(frustumHalfAngle);
        float dot = dot(frustumDirection, toFragNorm);
        inFrustum = (dot >= cosHalf) && (dist <= frustumRange);
    }

    if (!inFrustum) {
        // Desaturate
        float gray = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
        texColor.rgb = vec3(gray * 0.6); // darken slightly too
    }

    gl_FragColor = texColor;
}