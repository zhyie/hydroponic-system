async function loadMembers() {
    try {
    const response = await fetch('../docs/members.json');
    if (!response.ok) throw new Error('Network response was not ok');
    const members = await response.json();

    const container = document.getElementById('team-container');
    container.innerHTML = '';

    members.forEach(member => {
        const memberDiv = document.createElement('div');
        memberDiv.classList.add('member');

        memberDiv.innerHTML = `
        <div class="member-photo">
            <img src="${member.photo}" alt="Photo of ${member.name}">
        </div>
        <div class="member-details">
            <h2>${member.name}</h2>
            <p>${member.email}</p>
            <a href="${member.vitae}" target="_blank">View vitae</a>
        </div>
        `;

        container.appendChild(memberDiv);
    });
    } catch (error) {
    console.error('Failed to load members:', error);
    }
}

// Load members when the page loads
window.addEventListener('DOMContentLoaded', loadMembers);